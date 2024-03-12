DESCRIPTION = "Kernel loadable module for Videology gs-ar0234"

require ar0234.inc
inherit module

RRECOMMENDS:${PN} += "python3-ar0234"