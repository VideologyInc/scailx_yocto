Videology Yocto builder
=======================

# Build

## Prerequisites

- Linux PC (tested on `Ubuntu 22.04 LTS`) with [default host requirements](https://docs.yoctoproject.org/4.0.5/brief-yoctoprojectqs/index.html#build-host-packages)
- or use [docker](https://docs.docker.com/install/linux/docker-ce/ubuntu/)
- [kas](https://kas.readthedocs.io/): `sudo pip3 install kas`

## [Clone in VSCODE](vscode://vscode.git/clone?url=https%3A%2F%2Fgithub.com%2Fvideologyinc%2Fscailx_yocto.git)
## Native Linux Build using kas

```
# Clone
git clone git@github.com:VideologyInc/scailx_yocto.git
cd scailx_yocto

# build using kas
kas build
```

> [!NOTE]
> The initial build run **often fails** with a fetch error. this is because Github rate-limits the number of clone commands from a single PC. If you encounter this, simply run `kas build` again. Otherwise, run `kas shell` and in the bitbake environment, run `BB_NUMBER_THREADS=4 bitbake --runall fetch scailx-ml` to fetch everything before running the build again.

To enter the bitbake environment to perform other bitbake tasks, use `kas shell`:

```
kas shell
bitbake linux-imx scailx-ml <or some other recipe>
```

## Containerized Build
The easiest containerized build is by using [vscode devcontainers]([url](https://code.visualstudio.com/docs/devcontainers/containers)).

1. Make sure Docker is [installed and properly configured for your host system]([url](https://docs.docker.com/engine/install/)).
2. Clone the repo
3. Open in [visual-studio-code]([url](https://code.visualstudio.com/))
4. Open in devcontainer -> local
5. Run kas build as noted above.

> [!NOTE]
> Devcontainer build will not work under **Windows**, as devcontainers on windows mount the NTFS filesystem in the devcontainer, and this will fail with Yocto as it needs a journal-fs with permissions for OS creation. If you wish to build in Windows, use a [WSL2 device with ample vdisc space]([url](https://ejaaskel.dev/yocto-on-wsl2-easier-than-you-think/)) (>512GB) not mounted on your system (C:) drive.

## Update to the latest version
```
git stash
git switch -C master origin/master
kas build --force-checkout
```
