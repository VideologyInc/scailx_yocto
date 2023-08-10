require recipes-devtools/btrfs-tools/btrfs-tools_5.16.2.bb

PACKAGECONFIG ?= " \
    programs \
    dev \
    crypto-builtin \
"
