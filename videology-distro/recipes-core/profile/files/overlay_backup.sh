case $- in *i*)
    # check if 'recovery cmdline is set. then war user
    if grep -q recovery /proc/cmdline; then
        for i in $(cat /proc/cmdline); do
            case $i in
                bootslot=*) CURRENT_SLOT="${i#*=}" ;;
            esac
        done
        echo "~~ Boot failed. Fallback to working image at slot ${CURRENT_SLOT} ~~"
        echo "    Please update again with a working image "
    else
        # check if overlay backup exists
        if [ -d "/storage/overlay/backup" ] && [ ! -f "/storage/overlay/backup_ignore" ]; then
            echo " ~~ Update has removed User modified files from overlay ~~"
            echo "    Files moved to \"/storage/overlay/backup\". Review backup files, and copy them if necessary."
            echo "    Backup will be overwritten subsequent updates. Delete backup folder to remove this message."
            echo "    Put files you whish to keep permamnently in \"/storage/config/persist/<path/to/file>\""
            read -p "Delete/List/Ignore (D/L/I) backup?" choice
            case "$choice" in
                d|D ) rm -rf /storage/overlay/backup* ;;
                # a|A ) tar -cf - -C /storage/overlay/backup . | tar -xf - -C / && rm -rf /storage/overlay/backup ;;
                l|L ) find /storage/overlay/backup -type f ;;
                * ) echo "ignore" > "/storage/overlay/backup_ignore" ;;
            esac
        fi
    fi ;;
esac

