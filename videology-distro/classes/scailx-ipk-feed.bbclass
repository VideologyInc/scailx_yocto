# OPKG feeds generated from PACKAGE_FEED_URIS use "src/gz" and therefore
# require a compressed Packages index for every published architecture.
python do_package_index:append() {
    import gzip
    import os
    import shutil

    archs = set()
    for arch_var in ("ALL_MULTILIB_PACKAGE_ARCHS", "SDK_PACKAGE_ARCHS"):
        archs.update((d.getVar(arch_var) or "").split())

    for arch in archs:
        packages = os.path.join(d.getVar("DEPLOY_DIR_IPK"), arch, "Packages")
        if not os.path.isfile(packages):
            continue

        with open(packages, "rb") as source, open(packages + ".gz", "wb") as destination:
            with gzip.GzipFile(filename="", mode="wb", fileobj=destination, mtime=0) as compressed:
                shutil.copyfileobj(source, compressed)
}
