FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " file://defconfig"
SRC_URI += " file://add-devicetree-configfs.patch"

# remove the NXP preocesses that clobber the defconfig.
deltask copy_defconfig
deltask merge_delta_config


# do_compile:append() {
#     "source/scripts/clang-tools/gen_compile_commands.py" -o "source/compile_commands.json"
# }