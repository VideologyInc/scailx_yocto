from pathlib import Path


def get_mount_points():
    mounts = {}
    with open("/proc/self/mounts") as file:
        for line in file:
            dev, mp, *_ = line.split()
            mounts.setdefault(dev, []).append(mp)
    return mounts


mounts = get_mount_points()

for dev in Path("/sys/block").glob("mmcblk*"):
    try:
        dev_type = (dev / "device/type").read_text().strip()
        dev_nodes = [f"/dev/{dev.name}"]
        dev_nodes += [f"/dev/{p.name}" for p in dev.glob(f"{dev.name}p*")]

        mps = []
        for dn in dev_nodes:
            mps.extend(mounts.get(dn, []))

        mount_str = ", ".join(mps) if mps else "[unknown]"

        if dev_type == "SD":
            print(f"{dev.name} {mount_str}")
    except:
        pass
