#############################################################################
## U-Boot environment variables:
#############################################################################
SCAILX_UBOOT_ENV[scailx_bootcmd] ?= ' \
run default_bootargs;  \
if test -e ${devtype} ${devnum}:${distro_bootpart} boot.scr; then \
  load ${devtype} ${devnum}:${distro_bootpart} ${script_addr} boot.scr; \
  echo "Running distro script boot.scr"; \
  source ${script_addr}; \
else \
  run bootcmd_${boot_mode} bootm_cmd; \
fi'

SCAILX_UBOOT_ENV[bootcmd] = 'run scailx_bootcmd'
SCAILX_UBOOT_ENV[devtype] = 'mmc'
SCAILX_UBOOT_ENV[devnum] ?= '0'
SCAILX_UBOOT_ENV[distro_bootpart] ?= '1'
SCAILX_UBOOT_ENV[prefix] ?= ''
SCAILX_UBOOT_ENV[fdt_addr_r] ?= '40400000'
SCAILX_UBOOT_ENV[load_addr] ?= '40B00000'
SCAILX_UBOOT_ENV[script_addr] ?= '40D00000'
SCAILX_UBOOT_ENV[kernel_addr_r] ?= '41000000'
SCAILX_UBOOT_ENV[ramdisk_addr_r] ?= '47000000'
SCAILX_UBOOT_ENV[append_bootargs] ?= 'quiet audit=0'
SCAILX_UBOOT_ENV[console] ?= 'ttymxc0,115200 earlycon=ec_imx6q,0x30890000,115200'
SCAILX_UBOOT_ENV[default_bootargs] ?= 'setenv bootargs console=${console} panic=-1'
SCAILX_UBOOT_ENV[lock_hab] ?= 'no'
SCAILX_UBOOT_ENV[fuse_hab] ?= 'no'
SCAILX_UBOOT_ENV[mmcdev] ?= '0'

#############################################################################
