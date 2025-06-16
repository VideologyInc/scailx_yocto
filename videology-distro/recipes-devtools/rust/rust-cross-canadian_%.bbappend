do_install:append() {
    sed -ri 's:^(export CARGO_TARGET_'"${RUST_TARGET_TRIPLE}"'_RUSTFLAGS)="(.*)":\1="\2 -C link-arg=-Wl,--dynamic-linker=/usr/lib/ld-linux-aarch64.so.1":' ${RUST_ENV_SETUP_SH}
}