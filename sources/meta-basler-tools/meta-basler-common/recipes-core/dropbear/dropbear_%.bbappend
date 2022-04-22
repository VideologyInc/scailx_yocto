FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG[x11-forwarding]= ""

# activate x11 forwarding if the dropbear packageconfig is set in local.conf:
# PACKAGECONFIG_pn-dropbear_append = " x11_forwarding"
SRC_URI += "${@bb.utils.contains('PACKAGECONFIG', 'x11-forwarding', 'file://0001-enable-x11-forwarding.patch', '', d)} "

