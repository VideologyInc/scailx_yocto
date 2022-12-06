FROM gitpod/workspace-full

RUN sudo apt-get update \
 && sudo apt-get install -y \
    gawk wget git-core diffstat unzip texinfo \
    cpio screen bc tmux lz4 iproute2 zstd neofetch git-lfs bison tree \
    gcc-multilib build-essential chrpath socat cpio python python3 \
    python3-pip python3-pexpect xz-utils debianutils iputils-ping \
    libsdl1.2-dev xterm tar locales net-tools rsync sudo vim curl \
 && sudo rm -rf /var/lib/apt/lists/*