#! /usr/bin/env bash

if [[ $- == *i* ]]; then
    if [ -d "/storage/overlay/backup" ] && [ ! -f /storage/overlay/backup_ignore ]; then 
        echo " ~~ Update has removed User modified files from overlay ~~"
        echo "    Files moved to \"/storage/overlay/backup\". Review backup files, and copy them if necessary."
        echo "    Backup will be overwritten subsequent updates. Delete backup folder to remove this message."
        echo "    Put files you whish to keep permamnently in \"/storage/config/persist/<path/to/file>\""
        read -p "Delete/List/Apply/Ignore (D/A/L/I) backup?" choice
        case "$choice" in
            d|D ) rm -rf /storage/overlay/backup* ;;
            a|A ) tar -cf - -C /storage/overlay/backup . | tar -xf - -C / && rm -rf /storage/overlay/backup ;;
            l|L ) find /storage/overlay/backup -type f ;;
            i|I ) touch /storage/overlay/backup_ignore ;;
            * ) echo "";;
        esac
    fi
fi
