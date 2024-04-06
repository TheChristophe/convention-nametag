## Small nametag using Pi Zero W and various OLED panels

Work-in-progress self-programmed convetion nametag using a Raspberry Pi Zero W and a SSD1322 OLED panel. The intended
use is pinning it to a shirt or as a lanyard badge and uploading files using your phone.

#### Current state

- Recycled old gl nametag code
- Proof of concept done:

https://user-images.githubusercontent.com/65168240/141864525-b1e3a986-ae35-4b9f-8bda-cf15c19f3d5e.mp4


TODO:

- Playlist functionality
- Frontend application / website interface (using React)

### OLED Panels

Panels tried during development:

- [SH1106 HAT](https://www.waveshare.com/1.3inch-oled-hat.htm)
- [SSD1305 HAT](https://www.waveshare.com/2.23inch-oled-hat.htm)
- Soldered SSD1322
  using [this guide](https://www.balena.io/blog/build-a-raspberry-pi-powered-train-station-oled-sign-for-your-desk/)

### Frontend

[Repository](https://github.com/TheChristophe/convention-nametag-frontend)

### Backend

- Make sure to load git submodules: `git submodule --init --recursive`

If cross compiling:
- Set up cross-compilation
  - Follow the instructions in build-image to set up a `cross-compiler` image
- Start a container for compilation `docker run --rm -it -v .:/build/project cross-compiler`
- Build uSockets if you haven't yet: `cd include/uWebSockets/uSockets && CC=${TOOLDIR}/bin/${TOOLCHAIN}-gcc CXX=${TOOLDIR}/bin/${TOOLCHAIN}-g++ PREFIX=${TOOLDIR}/${TOOLCHAIN}/ CXXFLAGS=-I${TOOLDIR}/${TOOLCHAIN}/sysroot/usr/include/ LDFLAGS=-L${TOOLDIR}/${TOOLCHAIN}/sysroot/usr/lib make && cd ../../..`
- Build: `mkdir -p build && cd build && ../cross-cmake.sh .. && make`

else:

- Build uSockets if you haven't yet: `cd include/uWebSockets/uSockets && make`
- Build: `mkdir -p build && cd build && cmake .. && make`

Run with:

- `sudo ./build/nametag` (sudo due to GPIO permissions, unless you've handled those)

Notes:

- Ensure your video is already in desired size
    - From testing, h264 decode for 256x64 in software takes about ~5ms on pi zero. 480x360 requires about 33ms
    - In combination with 4.5ms copy time, this means <10ms, allowing for up to 100fps
