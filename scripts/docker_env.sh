#!/usr/bin/env bash
export DOCKER=docker
export DOCKER_REPO=yoedistro/yoe-build:bullseye

dkr() {
  # check_docker || return 1

  if [ -z "$1" ]; then
    echo "setting dkr action to shell"
    CMD="/bin/bash"
  else
    CMD="$@"
  fi

  touch ~/.gitconfig
  touch ~/.inputrc
  touch ~/.yocto_history

  if [ -e ~/.gitconfig ]; then
    MAP_GITCONFIG="--volume=$HOME/.gitconfig:/home/build/.gitconfig"
  fi

  GGID=$(id -g)
  UUID=$(id -u)
  UID_ARGS=""
  if [ "$DOCKER" = "podman" ]; then
    # https://github.com/containers/podman/issues/2180
    UID_ARGS="--privileged --uidmap $UUID:0:1 --uidmap 0:1:$UUID --gidmap $GGID:0:1 --gidmap 0:1:$GGID"
  fi

  $DOCKER run --rm -it \
    -v "$(pwd)":"$(pwd)" \
    -v ~/.ssh:/home/build/.ssh \
    -v ~/.gitconfig:/home/build/.gitconfig \
    -v ~/.inputrc:/home/build/.inputrc \
    -v ~/.~/.yocto_history:/home/build/.bash_history \
    $MAP_GITCONFIG \
    -w "$(pwd)" \
    $UID_ARGS --user=$UUID:$GGID \
    --cpu-shares=512 \
    ${DOCKER_REPO} /bin/bash -c "$CMD"
}

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
