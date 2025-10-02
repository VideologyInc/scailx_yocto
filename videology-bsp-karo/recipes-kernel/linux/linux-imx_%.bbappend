FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# remove the NXP preocesses that clobber the defconfig.
deltask copy_defconfig
deltask merge_delta_config

SRC_URI:append = " file://scailx-kmeta;type=kmeta;destsuffix=scailx-kmeta "

SRC_URI:append = " file://of-configfs.patch "
SRC_URI:append = " file://suppress-rcu-pointer.patch "
SRC_URI:append = " file://0001-ignore-imx-media-dev-probe-defer-if-no-sensor-i2c.patch "
SRC_URI:append = " file://0002-allow-userspace-usb-role-switch.patch "

PV:scailx = "${LINUX_VERSION}"

SCMVERSION = "n"
LINUX_VERSION_EXTENSION = ""

# generate compile_commands.json for clangd
do_compile:append() {
    # Generate compile_commands.json after kernel compilation
    bbnote "Generating compile_commands.json using kernel script"
    python3 ${S}/scripts/clang-tools/gen_compile_commands.py \
            -d ${B} \
            -o ${S}/compile_commands.json
}