FROM fedora:39

# dependencies
RUN dnf -y install \
    # for crosstool-ng
    git gcc g++ autoconf automake flex texinfo help2man patch libtool bison ncurses-devel bzip2 xz which unzip file python3 python3-devel diffutils wget  \
    # for building within the container
    cmake gdb && \
    # free up space
    dnf clean all && \
    rm -rf /var/cache/dnf


# crosstool-ng won't run as root
RUN useradd cross
USER cross
WORKDIR /build

# ct-ng config tool
RUN git clone https://github.com/crosstool-ng/crosstool-ng.git && \
    cd crosstool-ng && \
    bash ./bootstrap &&  \
    mkdir /build/ct-ng && \
    bash ./configure --prefix=/build/ct-ng && \
    make && \
    make install && \
    cd .. && \
    rm -rf crosstool-ng

# toolchain
ENV CT_PREFIX="/build/x-tools"
RUN /build/ct-ng/bin/ct-ng aarch64-rpi3-linux-gnu &&  \
    /build/ct-ng/bin/ct-ng build && \
    rm build.log

ENV TOOLCHAIN=aarch64-rpi3-linux-gnu
ENV TOOLDIR=/build/x-tools/aarch64-rpi3-linux-gnu

# TODO: why is this necessary and can it be configured otherwise
RUN chmod -R 755 ${TOOLDIR}/${TOOLCHAIN}/sysroot/usr

# ffmpeg
RUN wget https://ffmpeg.org/releases/ffmpeg-7.0.tar.xz && \
    tar -xf ffmpeg-7.0.tar.xz && \
    cd ffmpeg-7.0 && \
    ./configure --prefix=${TOOLDIR}/${TOOLCHAIN}/sysroot/usr --arch=aarch64 --target-os=linux --enable-cross-compile --cross-prefix=${TOOLDIR}/bin/${TOOLCHAIN}- && \
    make -j $(nproc) && \
    make install && \
    cd /build && \
    rm -rf ffmpeg-7.0 ffmpeg-7.0.tar.xz
# libbcm2835
RUN wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.75.tar.gz && \
    tar -xf bcm2835-1.75.tar.gz && \
    cd bcm2835-1.75 && \
    CC=${TOOLDIR}/bin/${TOOLCHAIN}-gcc CXX=${TOOLDIR}/bin/${TOOLCHAIN}-g++ ./configure --prefix=${TOOLDIR}/${TOOLCHAIN}/sysroot/usr --host=aarch64 && \
    make && \
    make install && \
    cd /build && \
    rm -rf bcm2835-1.75 bcm2835-1.75.tar.gz
# zlib
RUN wget https://www.zlib.net/zlib-1.3.1.tar.xz && \
    tar -xf zlib-1.3.1.tar.xz && \
    cd zlib-1.3.1 && \
    CC=${TOOLDIR}/bin/${TOOLCHAIN}-gcc CXX=${TOOLDIR}/bin/${TOOLCHAIN}-g++ ./configure --prefix=${TOOLDIR}/${TOOLCHAIN}/sysroot/usr && \
    make && \
    make install && \
    cd /build && \
    rm -rf zlib-1.3.1 zlib-1.3.1.tar.xz


WORKDIR /build
