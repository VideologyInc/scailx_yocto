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
    vvget_ctl.py -d /dev/video0 --save-default /opt/imx8-isp/bin/IMX678_Basic_1920x1080.xml
    vvget_ctl.py -d /dev/video0 --save-default IMX678_Basic_1920x1080.xml --set-ctrl brightness=-20 --restart-isp
    # live stream open: set via vvget AND persist to XML in one go
    vvget_ctl.py -d /dev/video0 --set-ctrl brightness=-20 --save-default IMX678_Basic_1920x1080.xml --apply-live

Notes:
    * vvget takes the numeric camera id (0 for /dev/video0), not the path.
    * imx controls come from the ISP, so vvget cannot report min/max/step;
      those ranges are defined in the CONTROLS table below and may need tuning
      against a real sensor.
    * --set-ctrl writes values straight through to vvget in the order given
      (explicit on/off toggles first). It does NOT force any ISP state, so for a
      manual value to take effect its gate must already be set: AEC/AWB OFF for
      manual exposure/gain/white-balance, CPROC ON for brightness/contrast/etc.
      Set those toggles yourself (same --set-ctrl works: auto_exposure=0,gain=5).
    * --set-ctrl writes are runtime-only and are lost when the ISP library
      reloads its baked defaults. To make a brightness (or any cproc) value the
      ISP's *default* -- surviving an ISP restart -- use --save-default to bake
      the cproc values into the sensor tuning XML's <cproc><config> block, which
      libcam_device.so loads at stream open. See
      docs/imx678-cproc-brightness-tuning.md for details.

"""

import argparse
import base64
import json
import os
import re
import struct
import subprocess
import sys


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
    # Ranges verified on imx900 hardware (2026-10): brightness -128 is rejected
    # (falls back to the baked default), so the usable floor is -127. hue is only
    # linear within [-90, 89] -- below -90 reads back 0, above 89 clamps to 89.
    # contrast/saturation span 0.0..1.99 and quantise to ~1/256 internally.
    {"id": "brightness", "window": "Adjust brightness", "type": "int",
     "group": "cproc", "min": -127, "max": 127, "step": 1, "default": 0},
    {"id": "contrast", "window": "Adjust contrast", "type": "float",
     "group": "cproc", "min": 0.0, "max": 1.99, "step": 0.01, "default": 1.0},
    {"id": "saturation", "window": "Adjust saturation", "type": "float",
     "group": "cproc", "min": 0.0, "max": 1.99, "step": 0.01, "default": 1.0},
    {"id": "hue", "window": "Adjust HUE", "type": "int",
     "group": "cproc", "min": -90, "max": 89, "step": 1, "default": 0},

    # --- Auto exposure (AEC) manual values ---
    # UNLIKE cproc, these ranges are NOT fixed: vvext reads them live from the ISP
    # per sensor/mode (gain -> EC_GAIN_MIN/MAX, exposure -> EC_INTEGRATION_MIN/MAX
    # divided by the HDR extension factor 2^extBit, iso -> AE_SENSITIVITY_MIN/MAX;
    # see isp-imx appshell/vvext/vvext.cpp PageAec*). --list-ctrls now parses the
    # live range out of vvget's prompt (see read_control/_extract_limits), so the
    # reported min/max reflect the connected sensor -- which differs between
    # imx662/678/900 and across modes. vvget only prints the range with a stream
    # OPEN and AEC OFF (otherwise it bails); the min/max below are the fallback for
    # that case, measured on imx900 (1920x1080@30, 2026-10-14). The ISP clamps any
    # out-of-range write to its own live min/max regardless.
    {"id": "gain", "window": "AEC Gain", "type": "float",
     "group": "aec", "min": 0.000000, "max": 0.000000, "step": 0.1, "default": 0.000000},
    {"id": "exposure_time", "window": "AEC ExposureTime", "type": "float",
     "group": "aec", "min": 0.000000, "max": 0.000000, "step": 0.000001, "default": 0.000000},
    {"id": "sensitivity", "window": "AEC Sensitivity", "type": "int",
     "group": "aec", "min": 100, "max": 1600, "step": 100, "default": 100},

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


# vvext prints the live AEC range inside the dump() input prompt, in three
# slightly different shapes (gain "[%f ~ %f]", exposure "(%f~%f)", iso
# "[%d ~ %d]"). This matches a "<min> ~ <max>" pair inside [] or ().
_LIMITS_RE = re.compile(r"[\[(]\s*(-?\d+(?:\.\d+)?)\s*~\s*(-?\d+(?:\.\d+)?)\s*[\])]")


def _extract_limits(stdout):
    """Parse a '[min ~ max]' / '(min~max)' range out of vvget stdout.

    AEC gain/exposure/iso ranges are not fixed: the ISP computes them live per
    sensor and mode, and vvext only exposes them in the prompt line of each
    control's dump (e.g. "please input new gain: [1.000000 ~ 2013.0]"). Returns
    (min, max) as floats, or (None, None) when no range is present -- vvget omits
    the prompt unless the stream is open AND AEC is off (otherwise it logs
    "aec is enabled, need disable first" and prints no range).
    """
    for line in stdout.splitlines():
        m = _LIMITS_RE.search(line)
        if m:
            return float(m.group(1)), float(m.group(2))
    return None, None


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


def read_control(cam_id, ctrl):
    """Read a control in one vvget call; return (value, min, max).

    For aec-group controls the min/max come back live from the ISP (parsed from
    vvget's prompt) instead of the static best-guess in CONTROLS, so --list-ctrls
    reports the true per-sensor range -- which now differs between imx662/678/900
    and across modes. When the live range is unavailable (stream closed or AEC
    on) we fall back to the table values. Non-aec controls always use the table.
    """
    stdout = _run(cam_id, ctrl["window"])
    raw = _extract_value(stdout, ctrl["window"])
    value = None if raw == "" else _cast(ctrl, raw)
    cmin, cmax = ctrl["min"], ctrl["max"]
    if ctrl["group"] == "aec":
        lo, hi = _extract_limits(stdout)
        if lo is not None:
            cmin, cmax = lo, hi
    return value, cmin, cmax


# ---------------------------------------------------------------------------
# operations (mirror v4l2-ctl)
# ---------------------------------------------------------------------------
def _control_item(cam_id, ctrl):
    """Build one Control[] entry (the portal's shape) for a control.

    Reads value + range in a single vvget call; aec controls get the live
    per-sensor min/max, every other control the static table range.
    """
    try:
        value, cmin, cmax = read_control(cam_id, ctrl)
    except RuntimeError:
        value, cmin, cmax = None, ctrl["min"], ctrl["max"]
    return {
        "control": ctrl["window"],
        "id": ctrl["id"],
        "minimum": cmin,
        "maximum": cmax,
        "step": ctrl["step"],
        "default": ctrl["default"],
        "value": value,
    }


def list_ctrls(cam_id, as_json=False):
    """--list-ctrls: dump every control with ranges + current value."""
    items = [_control_item(cam_id, ctrl) for ctrl in CONTROLS]

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


def get_ctrls(cam_id, ids, as_json=False):
    """--get-ctrl id[,id...]: print 'id: value' lines (v4l2-ctl format).

    With --json, emit the same Control[] shape as --list-ctrls but only for the
    requested ids -- crucially including the live per-sensor min/max for aec
    controls (parsed from the same vvget call, no extra latency). The portal's
    fast partial read uses this so the AEC sliders pick up their real range the
    moment Auto Exposure turns off, instead of a stale 0..0 fallback.
    """
    if as_json:
        items = []
        for cid in ids:
            ctrl = CONTROL_BY_ID.get(cid)
            if not ctrl:
                print(f"{cid}: unknown control", file=sys.stderr)
                continue
            items.append(_control_item(cam_id, ctrl))
        print(json.dumps(items, indent=2))
        return

    for cid in ids:
        ctrl = CONTROL_BY_ID.get(cid)
        if not ctrl:
            print(f"{cid}: unknown control", file=sys.stderr)
            continue
        print(f"{cid}: {get_control(cam_id, ctrl)}")


def set_ctrls(cam_id, pairs):
    """--set-ctrl id=value[,id=value...]: write each control directly.

    Values are passed straight through to vvget. Explicit on/off toggles are
    applied first so they gate their group's manual values (e.g. an explicit
    auto_exposure=0 before gain=5). Any required ISP state is the caller's
    responsibility -- manual AEC needs AEC off, color needs CPROC on, etc.
    """
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

    # Apply toggles first so an explicit on/off gates its group's manual values.
    parsed.sort(key=lambda cv: 0 if cv[0]["group"] == "toggle" else 1)
    for ctrl, value in parsed:
        try:
            _run(cam_id, ctrl["window"], value)
        except RuntimeError as e:
            print(f"failed to set {ctrl['id']}: {e}", file=sys.stderr)
            continue
        # vvget clamps writes to the ISP's live range, so read the value back and
        # report what actually took effect rather than the number we requested.
        try:
            applied = get_control(cam_id, ctrl)
        except RuntimeError:
            applied = None
        print(f"set {ctrl['id']} = {applied if applied is not None else value}")


# ---------------------------------------------------------------------------
# persistent cproc default (survive an ISP restart, not just a pipeline restart)
# ---------------------------------------------------------------------------
# --save / --load above round-trip runtime values through vvget, so they are lost
# whenever the ISP library reloads its baked defaults. To make a brightness (or any
# cproc) value the *default* the ISP comes up with, it has to go into the sensor
# tuning XML: camdev::Calibration::load() parses <matfile><tuning><cproc><config>
# and that <config> replaces the CalibCproc constructor default (baked brightness
# -15). See docs/imx678-cproc-brightness-tuning.md for the full reverse-engineering.
CPROC_FIELDS = ["brightness", "contrast", "saturation", "hue"]

AEC_FIELDS = ["gain","exposure_time", "sensitivity"]

# Baked CalibCproc defaults, used only as a fallback when a field can't be read
# live (cproc values are only readable while a stream is open) and wasn't supplied.
BAKED_CPROC = {"brightness": -15, "contrast": 1.1, "saturation": 1.0, "hue": 0.0}

ISP_SERVICE = "isp-imx-scailx.service"

# Match an existing <cproc ...>...</cproc> block (possibly multi-line), and the
# opening <tuning> tag we insert after when no <cproc> exists yet.
_CPROC_RE = re.compile(r"<cproc\b.*?</cproc>", re.DOTALL | re.IGNORECASE)
_TUNING_OPEN_RE = re.compile(r"<tuning\b[^>]*>", re.IGNORECASE)


def encode_cproc_config(brightness, contrast, saturation, hue):
    """Base64 of the 28-byte Vivante CamEngineCprocConfig_t.

    Layout (little-endian): chromaOut/lumaOut/lumaIn range enums (=2), contrast
    float, brightness int8 (+3 pad), saturation float, hue float.
    """
    payload = struct.pack(
        "<iii f b 3x f f",
        2, 2, 2,
        float(contrast),
        int(round(float(brightness))),
        float(saturation),
        float(hue),
    )
    return base64.b64encode(payload).decode("ascii")


def _collect_cproc_values(cam_id, overrides):
    """Resolve the four cproc fields: an override wins, else the live read, else
    the baked fallback (with a warning, since that usually means no stream open)."""
    values = {}
    for field in CPROC_FIELDS:
        if field in overrides:
            values[field] = overrides[field]
            continue
        try:
            live = get_control(cam_id, CONTROL_BY_ID[field])
        except RuntimeError:
            live = None
        if live is None:
            values[field] = BAKED_CPROC[field]
            print(
                f"warning: could not read live {field}; using fallback "
                f"{values[field]} (open a stream first to capture the live value, "
                f"or pass it via --set-ctrl)",
                file=sys.stderr,
            )
        else:
            values[field] = live
    return values


def _write_cproc_block(path, b64, enable=True):
    """Insert/replace the <cproc> block in a tuning XML, preserving the rest.

    Does a targeted string edit (not an XML re-serialise) so comments, formatting
    and sibling elements are left untouched. Keeps a one-time .bak-cproc backup.
    Returns 'replaced' or 'inserted'.
    """
    with open(path, "r", encoding="utf-8") as f:
        text = f.read()

    block = (
        f'<cproc enable="{"true" if enable else "false"}">'
        f"<config>{b64}</config></cproc>"
    )

    if _CPROC_RE.search(text):
        new_text = _CPROC_RE.sub(lambda _m: block, text, count=1)
        action = "replaced"
    else:
        m = _TUNING_OPEN_RE.search(text)
        if not m:
            raise RuntimeError(
                "no <tuning> section found; cannot insert <cproc> (is this the "
                "sensor tuning XML, e.g. IMX678_Basic_1920x1080.xml?)"
            )
        new_text = text[:m.end()] + "\n      " + block + text[m.end():]
        action = "inserted"

    backup = path + ".bak-cproc"
    if not os.path.exists(backup):
        with open(backup, "w", encoding="utf-8") as f:
            f.write(text)

    with open(path, "w", encoding="utf-8") as f:
        f.write(new_text)
    return action


def _parse_cproc_overrides(set_ctrl_arg):
    """Pull cproc field overrides out of a --set-ctrl string for --save-default.

    Non-cproc controls (aec/toggles) can't live in the <cproc> block, so they're
    ignored with a note rather than silently dropped.
    """
    overrides = {}
    for pair in [s.strip() for s in set_ctrl_arg.split(",") if s.strip()]:
        if "=" not in pair:
            print(f"Skipping malformed pair: {pair}", file=sys.stderr)
            continue
        cid, value = pair.split("=", 1)
        cid, value = cid.strip(), value.strip()
        ctrl = CONTROL_BY_ID.get(cid)
        if not ctrl or ctrl["group"] != "cproc":
            print(
                f"Ignoring non-cproc control for --save-default: {cid}",
                file=sys.stderr,
            )
            continue
        overrides[cid] = _cast(ctrl, value)
    return overrides


def save_default(cam_id, path, overrides, restart=False, service=ISP_SERVICE):
    """--save-default XML: bake the current cproc values into the tuning XML so the
    ISP comes up with them as its default (survives a pipeline/ISP restart).

    Values come from --set-ctrl overrides when given, otherwise the live device
    reads. Writes the <cproc><config> block and (optionally) restarts the ISP.
    """
    # Empty overrides => read every field live (used by --apply-live, where the
    # values were just pushed via vvget and the live read is the source of truth).
    values = _collect_cproc_values(cam_id, overrides)
    b64 = encode_cproc_config(
        values["brightness"], values["contrast"],
        values["saturation"], values["hue"],
    )
    try:
        action = _write_cproc_block(path, b64)
    except (OSError, RuntimeError) as e:
        print(f"failed to write {path}: {e}", file=sys.stderr)
        sys.exit(1)

    print(
        f"{action} <cproc> in {path}: brightness={values['brightness']} "
        f"contrast={values['contrast']} saturation={values['saturation']} "
        f"hue={values['hue']}"
    )
    print(f"  config base64: {b64}")

    if restart:
        print(
            f"restarting {service} (SSH may drop briefly; the device does not "
            f"reboot)..."
        )
        rc = subprocess.run(["systemctl", "restart", service]).returncode
        if rc != 0:
            print(f"warning: systemctl restart {service} exited {rc}", file=sys.stderr)
    else:
        print(
            f"  XML written but NOT yet loaded by the ISP (the live stream keeps "
            f"its current values until a restart).\n"
            f"  when ready, apply it manually with: systemctl restart {service}"
        )


# ---------------------------------------------------------------------------
# cli
# ---------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(
        description="v4l2-ctl-like control tool for Sony imx cameras via vvget, version 1.20",
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
        help="with --list-ctrls or --get-ctrl, emit JSON (portal Control[] shape, "
             "with the live per-sensor min/max for aec controls)",
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
        "--save-default", type=str, default="", metavar="XML",
        help="bake the current cproc values (brightness/contrast/saturation/hue) "
             "into a sensor tuning XML's <cproc> block so the ISP uses them as its "
             "default (survives an ISP restart). Reads live values unless overridden "
             "via --set-ctrl; use --restart-isp to apply immediately",
    )
    parser.add_argument(
        "--restart-isp", action="store_true",
        help="with --save-default: restart the ISP service after writing the XML",
    )
    parser.add_argument(
        "--isp-service", type=str, default=ISP_SERVICE, metavar="UNIT",
        help=f"systemd unit restarted by --restart-isp (default {ISP_SERVICE})",
    )
    parser.add_argument(
        "--apply-live", action="store_true",
        help="with --save-default + --set-ctrl: first apply the values live via "
             "vvget (visible immediately on the running stream), then snapshot the "
             "live result into the XML. Without this, --save-default only writes the "
             "XML and leaves the live stream untouched",
    )
    args = parser.parse_args()

    try:
        cam_id = device_to_id(args.device)
    except ValueError as e:
        print(e, file=sys.stderr)
        sys.exit(2)

    if args.save_default:
        if args.apply_live and args.set_ctrl:
            # Apply everything live first, then read the cproc values straight back
            # so the XML captures what actually stuck on the hardware rather than
            # the requested value.
            set_pairs = [s.strip() for s in args.set_ctrl.split(",") if s.strip()]
            set_ctrls(cam_id, set_pairs)
            overrides = {}
        else:
            overrides = _parse_cproc_overrides(args.set_ctrl)
        save_default(
            cam_id, args.save_default, overrides,
            restart=args.restart_isp, service=args.isp_service,
        )
    elif args.list_ctrls:
        list_ctrls(cam_id, args.json)
    elif args.get_ctrl:
        get_ctrls(
            cam_id,
            [s.strip() for s in args.get_ctrl.split(",") if s.strip()],
            args.json,
        )
    elif args.set_ctrl:
        set_ctrls(
            cam_id,
            [s.strip() for s in args.set_ctrl.split(",") if s.strip()],
        )
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
