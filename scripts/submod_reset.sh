#!/bin/bash
#Cleans and resets a git repo and its submodules
#https://gist.github.com/nicktoumpelis/11214362
git reset --hard
git submodule sync --recursive
git submodule update --init --force --recursive
git clean -ffdx
git submodule foreach --recursive git clean -ffdx
