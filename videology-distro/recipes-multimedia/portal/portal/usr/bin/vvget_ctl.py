#!/usr/bin/env python3

"""

File:   vvget_ctl.py

2026.0909.  Created a v4l2-ctl-like wrapper around vvget for Sony imx camera
            sensors (imx900 global shutter, imx678, imx662) that run through the
            Vivante ISP. Provides --list-ctrls / --get-ctrl / --set-ctrl so the
            portal can read and write imx controls the same way it uses v4l2-ctl
            for UVC/global-shutter cameras.

By:         mkkamarudin@videologyinc.com

Usage:
    vvget_ctl.py -d /dev/video0 --list-ctrls
    vvget_ctl.py -d /dev/video0 --list-ctrls --json
    vvget_ctl.py -d /dev/video0 --get-ctrl brightness
    vvget_ctl.py -d /dev/video0 --get-ctrl brightness,contrast
    vvget_ctl.py -d /dev/video0 --set-ctrl brightness=-20
    vvget_ctl.py -d /dev/video0 --set-ctrl brightness=-20,contrast=1.2
    vvget_ctl.py -d /dev/video0 --save imx0.json
    vvget_ctl.py -d /dev/video0 --load imx0.json
    vvget_ctl.py -d /dev/video0 --launch --load imx0.json
    vvget_ctl.py -d /dev/video0 --launch --load imx0.json --pipeline "v4l2src device=/dev/video0 ! ... ! websink"

Notes:
    * vvget takes the numeric camera id (0 for /dev/video0), not the path.
    * imx controls come from the ISP, so vvget cannot report min/max/step;
      those ranges are defined in the CONTROLS table below and may need tuning
      against a real sensor.
    * Auto Exposure (AEC) / Auto White Balance (AWB) must be OFF before manual
      values in their group take effect, and CPROC must be ON before the color
      adjustments apply. This script enforces that ordering automatically.
    * The Vivante ISP resets every control to its default whenever the v4l2
      pipeline (go2rtc / gst-launch) restarts. Use --save to snapshot the live
      values to a JSON file, and --load to re-apply that snapshot before the
      pipeline comes back up so user settings survive a restart.

"""

import argparse
import json
import shlex
import subprocess
import sys
import time


# ---------------------------------------------------------------------------
# Control metadata
# ---------------------------------------------------------------------------
# id     : v4l2-style lower_snake name exposed to callers / the portal.
# window : exact vvget "Window Name" (quoted on the command line).
# type   : 'int' | 'float' | 'bool'
# group  : 'cproc' -> needs CPROC ON before it applies.
#          'aec'   -> needs AEC On/Off = 0 before manual value applies.
#          'toggle'-> the on/off switch itself.
# min/max/step/default: ranges vvget can't report. Best-guess; verify on device.
CONTROLS = [
    # --- Color processing (CPROC) ---
    {"id": "brightness", "window": "Adjust brightness", "type": "int",
     "group": "cproc", "min": -128, "max": 127, "step": 1, "default": 0},
    {"id": "contrast", "window": "Adjust contrast", "type": "float",
     "group": "cproc", "min": 0.0, "max": 1.99, "step": 0.01, "default": 1.0},
    {"id": "saturation", "window": "Adjust saturation", "type": "float",
     "group": "cproc", "min": 0.0, "max": 1.99, "step": 0.01, "default": 1.0},
    {"id": "hue", "window": "Adjust HUE", "type": "int",
     "group": "cproc", "min": -128, "max": 127, "step": 1, "default": 0},

    # --- Auto exposure (AEC) manual values ---
    {"id": "gain", "window": "AEC Gain", "type": "float",
     "group": "aec", "min": 0.0, "max": 48.0, "step": 0.1, "default": 1.0},
    {"id": "exposure_time", "window": "AEC ExposureTime", "type": "float",
     "group": "aec", "min": 0.0, "max": 0.033, "step": 0.0001, "default": 0.009},
    {"id": "sensitivity", "window": "AEC Sensitivity", "type": "int",
     "group": "aec", "min": 100, "max": 3200, "step": 100, "default": 100},

    # --- Toggles ---
    {"id": "auto_exposure", "window": "AEC On/Off", "type": "bool",
     "group": "toggle", "min": 0, "max": 1, "step": 1, "default": 1},
    {"id": "auto_white_balance", "window": "AWB On/Off", "type": "bool",
     "group": "toggle", "min": 0, "max": 1, "step": 1, "default": 1},
    {"id": "color_processing", "window": "CPROC ON/OFF", "type": "bool",
     "group": "toggle", "min": 0, "max": 1, "step": 1, "default": 1},
    {"id": "gamma", "window": "GAMMA ON/OFF", "type": "bool",
     "group": "toggle", "min": 0, "max": 1, "step": 1, "default": 1},
    {"id": "denoise", "window": "DPF ON/OFF", "type": "bool",
     "group": "toggle", "min": 0, "max": 1, "step": 1, "default": 1},
]

CONTROL_BY_ID = {c["id"]: c for c in CONTROLS}

# vvget windows that gate a group.
AEC_ONOFF = "AEC On/Off"
AWB_ONOFF = "AWB On/Off"
CPROC_ONOFF = "CPROC ON/OFF"


# ---------------------------------------------------------------------------
# device / vvget plumbing
# ---------------------------------------------------------------------------
def device_to_id(device):
    """/dev/video0 -> 0 (the numeric id vvget expects)."""
    prefix = "/dev/video"
    if not device or not device.startswith(prefix):
        raise ValueError(f"Incorrect video device: {device}")
    try:
        return int(device[len(prefix):])
    except ValueError:
        raise ValueError(f"Incorrect video device: {device}")


def _run(cam_id, window, value=None):
    """Run vvget for one window (get if value is None, else set). Returns stdout.

    vvget usage is positional: `vvget <id> '<window>' '<param>'`. Two quirks of
    v1.00 handled here:
      * A value starting with '-' (negative brightness/hue, Offset INPUT vectors)
        is parsed as a flag and ignored. Prefixing it with a space dodges that;
        vvget's numeric parse strips the leading whitespace.
      * The 2-arg get form still prints an interactive prompt and reads stdin, so
        we feed it /dev/null to avoid hanging.
    """
    if value is None:
        cmd = f"vvget {cam_id} '{window}'"
    else:
        sval = str(value)
        if sval.startswith("-"):
            sval = " " + sval
        cmd = f"vvget {cam_id} '{window}' '{sval}'"
    result = subprocess.run(
        cmd, shell=True, capture_output=True, text=True, stdin=subprocess.DEVNULL
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"vvget failed ({cmd}): {result.stderr.strip() or result.stdout.strip()}"
        )
    return result.stdout


def _extract_value(stdout, window):
    """Pull the value out of vvget stdout for a given window name."""
    # Prefer the line that mentions the window name, else the first line with ':'.
    candidate = None
    for line in stdout.splitlines():
        if ":" not in line:
            continue
        if window.lower() in line.lower():
            candidate = line
            break
        if candidate is None:
            candidate = line
    if candidate is None:
        return ""
    return candidate.split(":", 1)[1].strip()


def _cast(ctrl, raw):
    """Cast a raw string value to the control's numeric type; fall back to raw."""
    raw = raw.strip().strip("[]")
    try:
        if ctrl["type"] == "int" or ctrl["type"] == "bool":
            return int(float(raw))
        if ctrl["type"] == "float":
            return float(raw)
    except (ValueError, TypeError):
        return raw
    return raw


def get_control(cam_id, ctrl):
    """Read one control's current value, cast to its type (None if unreadable).

    Some controls report nothing depending on state (e.g. manual AEC Gain while
    auto-exposure is on). Return None so callers/JSON emit null rather than ''.
    """
    stdout = _run(cam_id, ctrl["window"])
    raw = _extract_value(stdout, ctrl["window"])
    if raw == "":
        return None
    return _cast(ctrl, raw)


def _get_toggle(cam_id, window):
    """Read a raw on/off window as an int (0/1); -1 if unknown."""
    try:
        raw = _extract_value(_run(cam_id, window), window).strip().strip("[]")
        return int(float(raw))
    except (ValueError, RuntimeError, TypeError):
        return -1


def _ensure_off(cam_id, window, retries=10):
    """Turn an auto window off, polling until it reads 0 (AEC is unreliable)."""
    for _ in range(retries):
        if _get_toggle(cam_id, window) == 0:
            return True
        _run(cam_id, window, 0)
    return _get_toggle(cam_id, window) == 0


def _ensure_on(cam_id, window, retries=5):
    """Turn a window on, polling until it reads 1."""
    for _ in range(retries):
        if _get_toggle(cam_id, window) == 1:
            return True
        _run(cam_id, window, 1)
    return _get_toggle(cam_id, window) == 1


# ---------------------------------------------------------------------------
# operations (mirror v4l2-ctl)
# ---------------------------------------------------------------------------
def list_ctrls(cam_id, as_json=False):
    """--list-ctrls: dump every control with ranges + current value."""
    items = []
    for ctrl in CONTROLS:
        try:
            value = get_control(cam_id, ctrl)
        except RuntimeError:
            value = None
        items.append({
            "control": ctrl["window"],
            "id": ctrl["id"],
            "minimum": ctrl["min"],
            "maximum": ctrl["max"],
            "step": ctrl["step"],
            "default": ctrl["default"],
            "value": value,
        })

    if as_json:
        # Shape matches the portal's Control[] type for easy backend consumption.
        print(json.dumps(items, indent=2))
        return

    for it in items:
        print(
            f"{it['id']:<20} ({CONTROL_BY_ID[it['id']]['type']})".ljust(30)
            + f": min={it['minimum']} max={it['maximum']} step={it['step']} "
            + f"default={it['default']} value={it['value']}"
        )


def get_ctrls(cam_id, ids):
    """--get-ctrl id[,id...]: print 'id: value' lines (v4l2-ctl format)."""
    for cid in ids:
        ctrl = CONTROL_BY_ID.get(cid)
        if not ctrl:
            print(f"{cid}: unknown control", file=sys.stderr)
            continue
        print(f"{cid}: {get_control(cam_id, ctrl)}")


def set_ctrls(cam_id, pairs):
    """--set-ctrl id=value[,id=value...]: apply, honouring group ordering."""
    parsed = []
    for pair in pairs:
        if "=" not in pair:
            print(f"Skipping malformed pair: {pair}", file=sys.stderr)
            continue
        cid, value = pair.split("=", 1)
        cid, value = cid.strip(), value.strip()
        ctrl = CONTROL_BY_ID.get(cid)
        if not ctrl:
            print(f"Skipping unknown control: {cid}", file=sys.stderr)
            continue
        parsed.append((ctrl, value))

    if not parsed:
        return

    groups = {c["group"] for c, _ in parsed}
    explicit = {c["id"] for c, _ in parsed}

    # Pre-conditions, mirroring detect_imx_live.py ordering rules.
    if "cproc" in groups and "color_processing" not in explicit:
        _ensure_on(cam_id, CPROC_ONOFF)
    if "aec" in groups and "auto_exposure" not in explicit:
        # Manual AEC values only apply with auto exposure off.
        _ensure_off(cam_id, AEC_ONOFF)

    # Apply toggles first (so e.g. turning AWB off precedes its values), then rest.
    parsed.sort(key=lambda cv: 0 if cv[0]["group"] == "toggle" else 1)
    for ctrl, value in parsed:
        try:
            _run(cam_id, ctrl["window"], value)
            print(f"set {ctrl['id']} = {value}")
        except RuntimeError as e:
            print(f"failed to set {ctrl['id']}: {e}", file=sys.stderr)

    # Post-condition: the ISP 3A daemon can re-enable AEC *after* a manual value
    # write (auto exposure re-asserts its on/auto default), silently overriding
    # the value we just set. Re-assert AEC off once the writes are in.
    if "aec" in groups and "auto_exposure" not in explicit:
        _ensure_off(cam_id, AEC_ONOFF)


# ---------------------------------------------------------------------------
# persistence (survive a pipeline restart)
# ---------------------------------------------------------------------------
def save_ctrls(cam_id, path):
    """--save FILE: snapshot every readable control to a JSON {id: value} file.

    Written so --load can round-trip it straight back through set_ctrls. Controls
    that read back as None (e.g. manual AEC values while auto exposure is on) are
    skipped so we never persist a null the reload can't apply.
    """
    settings = {}
    for ctrl in CONTROLS:
        try:
            value = get_control(cam_id, ctrl)
        except RuntimeError:
            value = None
        if value is not None:
            settings[ctrl["id"]] = value

    try:
        with open(path, "w", encoding="utf-8") as f:
            json.dump(settings, f, indent=2)
            f.write("\n")
    except OSError as e:
        print(f"failed to save {path}: {e}", file=sys.stderr)
        sys.exit(1)
    print(f"saved {len(settings)} controls to {path}")


def load_ctrls(cam_id, path):
    """--load FILE: read a saved JSON {id: value} snapshot and re-apply it.

    Run this before (or right after) the v4l2 pipeline restarts to restore the
    user's settings on top of the ISP defaults. Reuses set_ctrls so the same
    AEC/AWB/CPROC ordering rules are honoured.
    """
    try:
        with open(path, "r", encoding="utf-8") as f:
            settings = json.load(f)
    except FileNotFoundError:
        print(f"load file not found: {path}", file=sys.stderr)
        sys.exit(1)
    except (OSError, json.JSONDecodeError) as e:
        print(f"failed to load {path}: {e}", file=sys.stderr)
        sys.exit(1)

    if not isinstance(settings, dict):
        print(f"unexpected format in {path}: expected a JSON object", file=sys.stderr)
        sys.exit(1)

    pairs = [f"{cid}={value}" for cid, value in settings.items()]
    set_ctrls(cam_id, pairs)


# ---------------------------------------------------------------------------
# launch: run a gstreamer pipeline and apply controls once the ISP is live
# ---------------------------------------------------------------------------
# The Vivante ISP only accepts control writes while something is actively pulling
# frames. Rather than poll for a stream we didn't start (detect_imx_live's model),
# this owns the pipeline: launch gst -> wait until it opens the device -> settle
# -> apply the snapshot/sets -> keep streaming in the foreground.
def _default_pipeline(device, width, height, fps):
    """A minimal NV12->H264 pipeline that keeps the ISP live (fakesink drops it).

    Pass your real pipeline via --pipeline for an actual sink (websink, udpsink,
    etc.); this default only exists to hold the stream open so controls apply.
    """
    return (
        f"v4l2src device={device} ! "
        f"video/x-raw,format=NV12,width={width},height={height},framerate={fps}/1 ! "
        f"imxvideoconvert_g2d ! queue ! vpuenc_h264 bitrate=3000 ! fakesink"
    )


def _device_open_by_gst(device):
    """True if a gst-launch process currently has the device open (ISP live)."""
    try:
        out = subprocess.run(
            ["lsof", device], capture_output=True, text=True
        ).stdout
    except FileNotFoundError:
        return False
    return "gst-launc" in out


def launch_and_apply(cam_id, device, pipeline, load_file, set_pairs,
                     settle=4.0, wait_open=15.0):
    """Run a gst-launch pipeline, apply controls once live, then stream.

    Returns the pipeline's exit code. Blocks in the foreground until the pipeline
    exits or Ctrl-C, forwarding termination to the child so nothing is orphaned.
    """
    cmd = ["gst-launch-1.0", "-q"] + shlex.split(pipeline)
    print(f"launching: {' '.join(shlex.quote(c) for c in cmd)}")
    try:
        proc = subprocess.Popen(cmd)
    except FileNotFoundError:
        print("gst-launch-1.0 not found on PATH", file=sys.stderr)
        return 1

    try:
        # Wait for the pipeline to open the device (ISP starts initialising).
        waited = 0.0
        opened = False
        while waited < wait_open:
            if proc.poll() is not None:
                print(
                    f"pipeline exited early (code {proc.returncode}) before going live",
                    file=sys.stderr,
                )
                return proc.returncode or 1
            if _device_open_by_gst(device):
                opened = True
                break
            time.sleep(0.5)
            waited += 0.5
        if not opened:
            print(
                "warning: could not confirm the pipeline opened the device; "
                "applying controls anyway",
                file=sys.stderr,
            )

        # Let the ISP settle so writes actually stick.
        time.sleep(settle)

        if set_pairs:
            set_ctrls(cam_id, set_pairs)
        if load_file:
            try:
                load_ctrls(cam_id, load_file)
            except SystemExit:
                # load_ctrls exits on a missing/bad file; keep streaming regardless.
                print(
                    f"load file '{load_file}' missing/invalid; streaming without it",
                    file=sys.stderr,
                )

        print("controls applied; streaming (Ctrl-C to stop)")
        proc.wait()
        return proc.returncode
    except KeyboardInterrupt:
        print("\nstopping pipeline...")
        return 0
    finally:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()


# ---------------------------------------------------------------------------
# cli
# ---------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(
        description="v4l2-ctl-like control tool for Sony imx cameras via vvget",
        prog="vvget_ctl",
    )
    parser.add_argument(
        "-d", "--device", type=str, default="/dev/video0",
        help="camera device path, e.g. /dev/video0",
    )
    parser.add_argument(
        "--list-ctrls", action="store_true",
        help="list all controls with ranges and current values",
    )
    parser.add_argument(
        "--json", action="store_true",
        help="with --list-ctrls, emit JSON (portal Control[] shape)",
    )
    parser.add_argument(
        "--get-ctrl", type=str, default="",
        help="get control value(s): id or id,id,...",
    )
    parser.add_argument(
        "--set-ctrl", type=str, default="",
        help="set control value(s): id=value or id=value,id=value,...",
    )
    parser.add_argument(
        "--save", type=str, default="", metavar="FILE",
        help="snapshot current control values to a JSON file",
    )
    parser.add_argument(
        "--load", type=str, default="", metavar="FILE",
        help="re-apply control values from a saved JSON file "
             "(run before the v4l2 pipeline restarts)",
    )
    parser.add_argument(
        "--launch", action="store_true",
        help="start a gstreamer pipeline, then apply --load/--set-ctrl once the "
             "ISP is live, and keep streaming until Ctrl-C",
    )
    parser.add_argument(
        "--pipeline", type=str, default="", metavar="GST",
        help="with --launch: gst-launch pipeline (args after 'gst-launch-1.0'); "
             "defaults to an NV12->H264->fakesink pipeline that just holds the ISP open",
    )
    parser.add_argument("--width", type=int, default=1280,
                        help="default-pipeline width (default 1280)")
    parser.add_argument("--height", type=int, default=720,
                        help="default-pipeline height (default 720)")
    parser.add_argument("--fps", type=int, default=30,
                        help="default-pipeline framerate (default 30)")
    parser.add_argument("--settle", type=float, default=4.0,
                        help="seconds to wait after the stream opens before "
                             "applying controls (default 4.0)")
    args = parser.parse_args()

    try:
        cam_id = device_to_id(args.device)
    except ValueError as e:
        print(e, file=sys.stderr)
        sys.exit(2)

    if args.launch:
        pipeline = args.pipeline or _default_pipeline(
            args.device, args.width, args.height, args.fps
        )
        set_pairs = [
            s.strip() for s in args.set_ctrl.split(",") if s.strip()
        ] if args.set_ctrl else []
        sys.exit(launch_and_apply(
            cam_id, args.device, pipeline, args.load, set_pairs, args.settle
        ))
    elif args.list_ctrls:
        list_ctrls(cam_id, args.json)
    elif args.get_ctrl:
        get_ctrls(cam_id, [s.strip() for s in args.get_ctrl.split(",") if s.strip()])
    elif args.set_ctrl:
        set_ctrls(cam_id, [s.strip() for s in args.set_ctrl.split(",") if s.strip()])
    elif args.save:
        save_ctrls(cam_id, args.save)
    elif args.load:
        load_ctrls(cam_id, args.load)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
