#!/bin/bash
#Cleans and resets a git repo and its submodules
#https://gist.github.com/nicktoumpelis/11214362

{
    set -e
    [ "$1" = "all" ] && git reset --hard
    git submodule sync --recursive
    git submodule update --init --force --recursive
    [ "$1" = "all" ] && git clean -ffdx
    git submodule foreach --recursive git clean -ffdx
}
if [ "$?" -eq 0 ]; then
    echo "git recursive submodule reset done."
else
    # unbinds all submodules
    git submodule deinit -f .
    # checkout again
    git submodule update --init --recursive
fi
