> ## NOTICE: these steps are for pi zero, outdated for pi zero 2
> I suggest using the docker image instead

Loose instructions to set up cross-compilation

0. Required packages: Arch: `base-devel`, `help2man`, `rsync`, `unzip`

1. `git clone https://github.com/crosstool-ng/crosstool-ng`, `cd crosstool-ng`
2. Generate ./configure: `./bootstrap`
3. Add folder for crosstool: `mkdir -p $HOME/.local/crosstool-ng`
4. `./configure --prefix=$HOME/.local/crosstool-ng`
5. Build: `make`, `make install`
6. Add to path: `export PATH=$PATH:$HOME/.local/crosstool-ng/bin/`
7. Make a folder to build toolchain in: `mkdir ~/pi-toolchain`, `cd ~/pi-toolchain`
8. Use rpi preset: `ct-ng armv6-unknown-linux-gnueabihf`
9. Use `ct-ng menuconfig`, select recent enough (not newer than on your rpi!) linux, binutils, glibc, gcc version
10. Build: `ct-ng build` (~15 min if no errors)
11. Download ffmpeg: `git clone https://git.ffmpeg.org/ffmpeg.git`, `cd ffmpeg`
12. Build ffmpeg: `CC=${TOOLDIR}/bin/${TOOLCHAIN}-gcc CXX=${TOOLDIR}/bin/${TOOLCHAIN}-g++ ./configure --prefix=${TOOLDIR}/${TOOLCHAIN}/sysroot/usr --arch=armel --target-os=linux --enable-cross-compile --cross-prefix=${TOOLDIR}/bin/${TOOLCHAIN}-`, `make`, `make install`
13. Build uSockets: `cd include/uWebSockets/uSockets`, `CC=${TOOLDIR}/bin/${TOOLCHAIN}-gcc CXX=${TOOLDIR}/bin/${TOOLCHAIN}-g++ PREFIX=${TOOLDIR}/${TOOLCHAIN}/ CXXFLAGS=-I${TOOLDIR}/${TOOLCHAIN}/sysroot/usr/include/ LDFLAGS=-L${TOOLDIR}/${TOOLCHAIN}/sysroot/usr/lib make`
14. Build zlib: `https://www.zlib.net/`, `CC=${TOOLDIR}/bin/${TOOLCHAIN}-gcc CXX=${TOOLDIR}/bin/${TOOLCHAIN}-g++ ./configure --prefix=${TOOLDIR}/${TOOLCHAIN}/sysroot/usr`
15. Download & extract to sysroot: https://archlinuxarm.org/packages/armv6h/libbcm2835
16. You should now be ready to go
