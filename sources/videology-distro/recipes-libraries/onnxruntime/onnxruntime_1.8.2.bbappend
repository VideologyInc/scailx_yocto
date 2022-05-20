do_install_append() {
    for f in `find ${D}/${PYTHON_SITEPACKAGES_DIR}/bin -type f -not -type l`; do
        sed -i -e 's:^#!.*nativepython:#!/usr/bin/env python:g' $f;
    done
}
