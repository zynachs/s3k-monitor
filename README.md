# s3k-monitor

## Project in DD2497 

This repository is a project in the course DD2497 (Project course in System Security) given by [KTH](https://www.kth.se/student/kurser/kurs/DD2497).

The project's goal is to implement security features into [kth-step/s3k](https://github.com/kth-step/s3k)  

## Project members

* Zacharias Terdin (<zacte@kth.se>)
* Elin Kårehagen (<elikare@kth.se>)

## Table of Contents
- [s3k-monitor](#s3k-monitor)
  - [Project in DD2497](#project-in-dd2497)
  - [Project members](#project-members)
  - [Table of Contents](#table-of-contents)
- [Documentation](#documentation)
  - [Overview](#overview)
  - [Repository layout](#repository-layout)
    - [Notable folders and files:](#notable-folders-and-files)
  - [Dependencies and compatibility](#dependencies-and-compatibility)
    - [Required software:](#required-software)
  - [Compiling and running](#compiling-and-running)
  - [Memory layout](#memory-layout)
---

# Documentation


## Overview

This repository consists of mainly two applications: *monitor* and *app*. The monitor is the core of this repository while the app is an application which will be used to test the functionality of the monitor. 

>The purpose of the monitor is to be the first user-level process in the s3k kernel which will be responsible for *monitoring* all other user processes. The responsibilities of the monitor will be to authenticate the code of other processes, load the processes into memory, handling their capabilities to time and memory and handle certain exceptions.

**For information about the monitor see [project-proposal.md](./project-proposal.md)**


## Repository layout

### Notable folders and files:

| Folder/File | Description |
| --- | --- |
| [app/](./app/) | Contains files which are unique to app. |
| [app/app.c](./app/app.c) | Source file. |
| [app/app.lds](./app/app.lds) | Linker script. |
| [common/](./common/) | Contains assembly files which are shared between monitor and app. | 
| [inc/](./inc/) | Contains header files which are shared between monitor and app. | 
| [lib/](./lib/) | Contains header archive files which are shared between monitor and app. | 
| [misc/config.h](./misc/config.h) | Header file for compiling the kernel |
| [misc/default.ld](./misc/default.ld) | Default linker script. Does not comply with PMP. |
| [monitor/](./monitor/) | Contains files which are unique to monitor |
| [monitor/monitor.c](./monitor/monitor.c) | Source file. |
| [monitor/monitor.lds](./monitor/monitor.lds) | Linker script. |
| [scripts/](./scripts/) | Contains helpful tools |
| [scripts/payloads/](./scripts/payloads/) | Contains C source files which are used by genpayload.sh to create raw machine code. |
| [scripts/breakpoints.txt](./scripts/breakpoints.txt) | Text file containing one breakpoint per line GDB which is executed in qemu.sh |
| [scripts/genpayload.sh](./scripts/genpayload.sh) | Bash script which generates a comma-seperated list of raw machine code in 0x notation from C source files in scripts/payloads/. |
| [scripts/qemu.sh](./scripts/qemu.sh) | Bash script which emulates the kernel, monitor and app with qemu and spawns a gdb live debugging prompt. Utilizes tmux.|
| [scripts/riscvpmp.py](./scripts/riscvpmp.py) | Python script which translates a pmpaddr to start and end address. |
| [scripts/riscvpmpinv.py](./scripts/riscvpmpinv.py) | Python script which translates a start and end address to a pmpaddr. |
| [Makefile](./Makefile) | Root Makefile, includes all other makefiles. Used to compile and execute the repository files |
| [project-proposal.md](./project-proposal.md) | Original project proposal, 2023-03-02. |


## Dependencies and compatibility

> The development environment for this repository is Ubuntu Linux on [WSL](https://learn.microsoft.com/en-us/windows/wsl/install). It should be compatible with most Linux distributions on bare-metal installations or virtual machines.

### Required software:

| Software | Version | Source | Note | 
| --- | --- | --- | --- | 
| git | N/A | https://git-scm.com/ | - | 
| GNU Make | N/A | https://www.gnu.org/software/make/ | - |
| RISC-V GNU Compiler Toolchain | tag: 2023.02.21 | https://github.com/riscv-collab/riscv-gnu-toolchain/releases/tag/2023.02.21 | Only tested the specified version and compiled from source. Version in package manager might be outdated. |
| QEMU | Release: 7.2.0 | https://github.com/qemu/qemu/releases/tag/v7.2.0 | Only tested the specified version and compiled from source. Version in package manager might be outdated. |
| kth-step/s3k | commit: c0b7800 | https://github.com/kth-step/s3k/commit/c0b78005261f4725be71e103f70e61b0a8a2fee2 | Only tested and compatible with the specified commit of s3k. |


## Compiling and running

> This repository **DEPENDS** on and expects that it resides in the same parent folder as [kth-step/s3k](https://github.com/kth-step/s3k).

1. Clone both repositories.
2. Checkout step-kth/s3k to commit `c0b78005261f4725be71e103f70e61b0a8a2fee2`
3. Make sure that they are in the same parent folder.
4. Cd into s3k-monitor

```shell
git clone https://github.com/zynachs/s3k-monitor.git
git clone https://github.com/kth-step/s3k.git
cd s3k
git checkout c0b7800
cd ../s3k-monitor
```

4. Install prerequisite packages and tools, build from source or install from package manager. The links below are to the sources. the code block is an example of installing with the apt package manager on Ubuntu.
   - [RISC-V GNU Compiler Toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain.git)
   - [QEMU](https://github.com/qemu/qemu)

```shell
sudo apt update
sudo apt install -y gcc-riscv64-unknown-elf qemu-system-misc
```

5. Compile kernel, monitor and app.

```shell
make
```
6. Execute kernel, monitor and app with qemu and launch a GDB live debugging session.

```shell
make qemu
```

7. In a separate terminal read output by connecting with netcat or telnet.

```shell
nc -v localhost 4321
telnet localhost 4321
```

8. Add debugging information during execution.
```shell
make clean_repo
make CLIFLAG=debug
make qemu
```

9. Execute test (existing tests: 1, 2, 3).
```shell
make clean_repo
make CLIFLAG=testX
make qemu
```

## Memory layout

The kernel does not use virtual memory and does not have an MMU. It uses physical memory addresses and MPU (Memory Protection Unit).

>The s3k kernel is emulated on the builtin *virt* hardware board. The following information is dependant on this hardware.

```
                                            ┌─────────────┬─────────┐
                                            │   Address   │ Section │
                                            ├─────────────┼─────────┤
                                           >│ 0x8003_0000 │ ...     │
                                          />│ [2K]        │ .stack  │
┌─────────────┬────────────────────┐     / /│ 0x8001_F800 │ .stack  │
│   Address   │       Memory       │    / / │ ...         │ ...     │
├─────────────┼────────────────────┤   / /  │ 0x8002_3000 │ ...     │
│ 0x8800_0000 │ ... (RAM END)      │  / /   │ [4K]        │ .bss    │
│ ...         │ ...                │ / /    │ 0x8002_2000 │ .bss    │
│ 0x8003_0000 │ ...                │/ /     │ [2K]        │ .rodata │
│ [64K]       │ APP ---------------> /      │ 0x8002_1800 │ .rodata │
│ 0x8002_0000 │ APP --------------->/       │ [2K]        │ .data   │
│ [64K]       │ MONITOR ----------->\       │ 0x8002_1000 │ .data   │
│ 0x8001_0000 │ MONITOR -----------> \      │ [4K]        │ .text   │ 
│ [64K]       │ KERNEL             │\ \     │ 0x8002_0000 │ .text   │ 
│ 0x8000_0000 │ KERNEL (RAM START) │ \ \    └─────────────┴─────────┘ 
│ ...         │ ...                │  \ \   ┌─────────────┬─────────┐
│ 0x1000_0100 │ ...                │   \ \  │   Address   │ Section │
│ [256B]      │ UART               │    \ \ ├─────────────┼─────────┤
│ 0x1000_0000 │ UART               │     \ >│ 0x8002_0000 │ ...     │
│ ...         │ ...                │      \>│ [2K]        │ .stack  │      
│ 0x0000_0000 │ ...                │        │ 0x8001_F800 │ .stack  │
└─────────────┴────────────────────┘        │ ...         │ ...     │
                                            │ 0x8001_9000 │ ...     │
                                            │ [4K]        │ .bss    │
                                            │ 0x8001_8000 │ .bss    │
                                            │ [16K]       │ .rodata │
                                            │ 0x8001_4000 │ .rodata │
                                            │ [8K]        │ .data   │
                                            │ 0x8001_2000 │ .data   │
                                            │ [8K]        │ .text   │
                                            │ 0x8001_0000 │ .text   │
                                            └─────────────┴─────────┘