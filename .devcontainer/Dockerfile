FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install gawk wget git diffstat unzip texinfo gcc build-essential chrpath socat cpio python3 python3-pip \
    python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev pylint xterm \
    python3-subunit mesa-common-dev zstd liblz4-tool kas file

# Yocto needs 'source' command for setting up the build environment, so replace
# the 'sh' alias to 'bash' instead of 'dash'.
RUN rm /bin/sh && ln -s bash /bin/sh

# add bash indexed search
RUN echo '"\e[A": history-search-backward' >> .inputrc && \
	echo '"\e[B": history-search-forward'  >> .inputrc && \
	echo '"\e[C": forward-char'            >> .inputrc && \
	echo '"\e[D": backward-char'           >> .inputrc
