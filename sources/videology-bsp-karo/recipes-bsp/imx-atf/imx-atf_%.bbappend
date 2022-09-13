FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-2.2:"

ATF_SRC = "git://github.com/karo-electronics/imx-atf.git;protocol=https"
SRCBRANCH = "lf_v2.4-karo"
SRCREV = "1d0298da0f9ec51fa11345811ad314244bf075b9"

EXTRA_OEMAKE += " \
        IMX_BOOT_UART_BASE=${IMX_BOOT_UART_BASE} ERRATA_A53_1530924=1 \
"
