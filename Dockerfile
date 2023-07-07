FROM ubuntu:latest

ARG GNU_TOOLCHAIN_VERSION=2023.02.21
ARG QEMU_VERSION=v7.2.0
ENV PATH "$PATH:/opt/riscv/bin:/opt/qemu/bin"

RUN apt-get update -y && apt-get install -y \
# GNU toolchain prerequisites
    autoconf \
    automake \
    autotools-dev \
    curl \
    python3 \
    python3-pip \
    libmpc-dev \
    libmpfr-dev \
	libgmp-dev \
	gawk \
	build-essential \
	bison \
	flex \
	texinfo \
	gperf \
	libtool \
	patchutils \
	bc \
	zlib1g-dev \
	libexpat-dev \
	ninja-build \
	git \
	cmake \
	libglib2.0-dev \
    libncurses5-dev \
# QEMU prerequisites
    git \
    libglib2.0-dev \
    libfdt-dev \
    libpixman-1-dev \
    zlib1g-dev \
    ninja-build \
# QEMU recommended additional packages
    git-email \
	libaio-dev \
	libbluetooth-dev \
	libcapstone-dev \
	libbrlapi-dev \
	libbz2-dev \
	libcap-ng-dev \
	libcurl4-gnutls-dev \
	libgtk-3-dev \
	libibverbs-dev \
	libjpeg8-dev \
	libncurses5-dev \
	libnuma-dev \
	librbd-dev \
	librdmacm-dev \
	libsasl2-dev \
	libsdl2-dev \
	libseccomp-dev \
	libsnappy-dev \
	libssh-dev \
	libvde-dev \
	libvdeplug-dev \
	libvte-2.91-dev \
	libxen-dev \
	liblzo2-dev \
	valgrind \
	xfslibs-dev \
    libnfs-dev \
    libiscsi-dev \
# s3k-monitor prerequisites
    tmux \
    bsdmainutils

RUN pip install pyelftools

RUN mkdir -p /work/

WORKDIR /work

RUN git clone https://github.com/riscv/riscv-gnu-toolchain
WORKDIR /work/riscv-gnu-toolchain
RUN git switch --detach tags/${GNU_TOOLCHAIN_VERSION}
RUN mkdir -p /opt/riscv
RUN ./configure --prefix=/opt/riscv --enable-multilib
RUN make -j$(nproc)

WORKDIR /work

RUN git clone https://github.com/qemu/qemu.git
WORKDIR /work/qemu
RUN git switch --detach tags/${QEMU_VERSION}
RUN mkdir -p /opt/qemu/bin
WORKDIR /opt/qemu/bin
RUN /work/qemu/configure --target-list=riscv64-softmmu
RUN make -j$(nproc)

RUN rm -rf /work/*
RUN mkdir -p /work/s3k-monitor

WORKDIR /work/s3k-monitor

CMD ["options"]

ENTRYPOINT ["make"]
