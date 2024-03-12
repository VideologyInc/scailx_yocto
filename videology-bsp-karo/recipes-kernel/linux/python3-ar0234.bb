DESCRIPTION = "Python helper scripts for the Videology gs-ar0234"

require ar0234.inc
inherit setuptools3

RDEPENDS:${PN} += "python3-periphery"
