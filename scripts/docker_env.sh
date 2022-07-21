#!/usr/bin/env bash
export DOCKER=docker
export DOCKER_REPO=yoedistro/yoe-build:bullseye

DOCKER_IMAGE=yocto_builder

SOURCED=0
[[ "${BASH_SOURCE[0]}" != "${0}" ]] && SOURCED=1
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

docker_build() {
  echo "building ${SCRIPT_DIR}/Dockerfile"
  docker build  -t "${DOCKER_IMAGE}" \
                --build-arg "USER=$(whoami)" \
                --build-arg "host_uid=$(id -u)" \
                --build-arg "host_gid=$(id -g)" \
                "${SCRIPT_DIR}"
}

check_docker() {
  docker image inspect -f ' ' "$DOCKER_IMAGE" || docker_build
}

dkr() {
  check_docker || return 1

  if [ -z "$1" ]; then
    echo "setting dkr action to shell"
    CMD="/bin/bash"
  else
    CMD="$@"
  fi

  [ -e ~/.gitconfig ]     || touch ~/.gitconfig
  [ -e ~/.inputrc ]       || touch ~/.inputrc
  [ -e ~/.yocto_history ] || touch ~/.yocto_history

  GGID=$(id -g)
  UUID=$(id -u)
  USER=$(whoami)
  UID_ARGS=""
  if [ "$DOCKER" = "podman" ]; then
    # https://github.com/containers/podman/issues/2180
    UID_ARGS="--privileged --uidmap $UUID:0:1 --uidmap 0:1:$UUID --gidmap $GGID:0:1 --gidmap 0:1:$GGID"
  fi

  $DOCKER run --rm -it \
    -v "$(pwd)":"$(pwd)" \
    -v ~/.ssh:/home/"${USER}"/.ssh \
    -v ~/.gitconfig:/home/"${USER}"/.gitconfig \
    -v ~/.inputrc:/home/"${USER}"/.inputrc \
    -v ~/.yocto_history:/home/"${USER}"/.bash_history \
    -w "$(pwd)" \
    -e "TERM=xterm-256color" \
    $UID_ARGS --user=$UUID:$GGID \
    --cpu-shares=512 \
    ${DOCKER_IMAGE} nice -n 10 /bin/bash -c "$CMD"
}

[ "$SOURCED" = "1" ] || dkr "$@"

# bitbake() {
#   ulimit -n 4096
#   if [ -z $DOCKER_REPO ] || [ "$DOCKER_REPO" = "none" ]; then
#     "$(pwd)"/sources/poky/bitbake/bin/bitbake $@
#   else
#     dkr "$(pwd)/sources/poky/bitbake/bin/bitbake $@"
#   fi
# }

# bitbake-layers() {
#   ulimit -n 4096
#   if [ -z $DOCKER_REPO ] || [ "$DOCKER_REPO" = "none" ]; then
#     "$(pwd)"/sources/poky/bitbake/bin/bitbake-layers $@
#   else
#     dkr "$(pwd)/sources/poky/bitbake/bin/bitbake-layers $@"
#   fi
# }

# devtool() {
#   ulimit -n 4096
#   if [ -z $DOCKER_REPO ] || [ "$DOCKER_REPO" = "none" ]; then
#     "$(pwd)"/sources/poky/scripts/devtool $@
#   else
#     dkr "$(pwd)/sources/poky/scripts/devtool $@"
#   fi
# }
