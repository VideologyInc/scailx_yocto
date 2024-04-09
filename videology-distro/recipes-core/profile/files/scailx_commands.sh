# /usr/bin/env sh

persist() {
    files=$(realpath $@)
    for f in $files; do
        fdirs=$(dirname "/storage/config/persist$f")
        mkdir -p "$fdirs"
        cp -rf "/storage/overlay/upper$f" "/storage/config/persist$f" || true
    done
}

ov_restore() {
    files=$(realpath $@)
    for f in $files; do
        cp -rf "/storage/overlay/backup$f" "$f" || true
    done
}

