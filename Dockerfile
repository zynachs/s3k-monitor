# Builds docker image to compile and run s3k-monitor

#
# Stage 0: build RISC-V GNU Toolchain & QEMU
#

FROM ubuntu:latest
ENV PATH "$PATH:/opt/riscv/bin:/opt/qemu/bin"
WORKDIR /work
ARG GNU_TOOLCHAIN_VERSION=2023.02.21
ARG QEMU_VERSION=v7.2.0

# RISC-V GNU Toolchain prerequisites
RUN apt-get update -y && apt-get install -y \
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
    libncurses5-dev

# Build RISC-V GNU Toolchain
RUN git clone https://github.com/riscv/riscv-gnu-toolchain
WORKDIR /work/riscv-gnu-toolchain
RUN git switch --detach tags/${GNU_TOOLCHAIN_VERSION}
RUN mkdir -p /opt/riscv
RUN ./configure --prefix=/opt/riscv --enable-multilib
RUN make -j$(nproc)

WORKDIR /work

# QEMU prerequisites
RUN apt update -y && apt install -y \
	make \
	git \
    libglib2.0-dev \
    libfdt-dev \ 
    libpixman-1-dev \
    zlib1g-dev \
    ninja-build

# Build QEMU
RUN git clone https://github.com/qemu/qemu.git
WORKDIR /work/qemu
RUN git switch --detach tags/${QEMU_VERSION}
RUN mkdir -p /opt/qemu/bin
WORKDIR /opt/qemu/bin
RUN /work/qemu/configure --target-list=riscv64-softmmu 
RUN make -j$(nproc)

#
# Stage 1: Assemble light weight image  
#

FROM ubuntu:latest
ENV PATH "$PATH:/opt/riscv/bin:/opt/qemu/bin"
WORKDIR /work/s3k-monitor

# s3k-monitor prerequisites
RUN apt update -y && apt-get install -y \
	# Requirements for qemu
    libglib2.0-dev \
    libfdt-dev \ 
    libpixman-1-dev \
	# Requirements for s3k-monitor
	python3 \
	python3-pip \
    tmux \
    bsdmainutils \
	less \
	make

# Required for s3k-monitor
RUN pip install pyelftools

# Copy binaries built in stage 0
COPY --from=0 /opt/ /opt/

# Default argument
CMD ["options"]

# Default command
ENTRYPOINT ["make"]
