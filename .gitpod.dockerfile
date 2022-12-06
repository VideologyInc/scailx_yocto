FROM gitpod/workspace-full

RUN sudo apt-get update \
 && sudo apt-get install -y \
    gawk wget git-core diffstat unzip texinfo \
    cpio screen bc tmux lz4 iproute2 zstd neofetch git-lfs bison tree \
    gcc-multilib build-essential chrpath socat cpio python python3 \
    python3-pip python3-pexpect xz-utils debianutils iputils-ping \
    libsdl1.2-dev xterm tar locales net-tools rsync sudo vim curl \
 && sudo rm -rf /var/lib/apt/lists/*

 RUN sudo pip3 install --no-input kas

 # add bash indexed search
RUN echo '"\e[A": history-search-backward' >> .inputrc && \
    echo '"\e[B": history-search-forward'  >> .inputrc && \
    echo '"\e[C": forward-char'            >> .inputrc && \
    echo '"\e[D": backward-char'           >> .inputrc

# Yocto needs 'source' command for setting up the build environment, so replace
# the 'sh' alias to 'bash' instead of 'dash'.
RUN sudo rm /bin/sh && sudo ln -s bash /bin/sh
