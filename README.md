# s3k-monitor

## Project in DD2497 

This repository is a project in the course DD2497 (Project course in System Security) given by [KTH](https://www.kth.se/student/kurser/kurs/DD2497).

The project's goal is to implement security features into [kth-step/s3k](https://github.com/kth-step/s3k)  

## Project members

* Zacharias Terdin (<zacte@kth.se>)
* Elin Kårehagen (<elikare@kth.se>)

# Documentation

## Table of Contents
- [s3k-monitor](#s3k-monitor)
  - [Project in DD2497](#project-in-dd2497)
  - [Project members](#project-members)
- [Documentation](#documentation)
  - [Table of Contents](#table-of-contents)
  - [Overview](#overview)
  - [Repository layout](#repository-layout)
    - [Notable folders and files:](#notable-folders-and-files)
  - [Instructions for compiling and running](#instructions-for-compiling-and-running)
  - [Memory Layout](#memory-layout)
---

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
| [common/](./common/) | Contains source files, header files and assembly files which are shared between monitor and app. | 
| [monitor/](./monitor/) | Contains files which are unique to monitor |
| [monitor/monitor.c](./monitor/monitor.c) | Source file. |
| [monitor/monitor.lds](./monitor/monitor.lds) | Linker script. |
| [scripts/](./scripts/) | Contains helpful tools |
| [scripts/payloads/](./scripts/payloads/) | Contains C source files which are used by genpayload.sh to create raw machine code. |
| [scripts/genpayload.sh](./scripts/genpayload.sh) | Bash script which generates a comma-seperated list of raw machine code in 0x notation from a C source file. |
| [scripts/qemu.sh](./scripts/qemu.sh) | Bash script which emulates the kernel, monitor and app with qemu and spawns a gdb live debugging prompt. |
| [scripts/riscvpmp.py](./scripts/riscvpmp.py) | Python script which translates a pmpaddr to start and end address. |
| [scripts/riscvpmpinv.py](./scripts/riscvpmpinv.py) | Python script which translates a start and end address to a pmpaddr. |
| [config.h](./config.h) | Header file for compiling the kernel |
| [config.mk](./config.mk) | Configuration file for all Makefiles |
| [Makefile](./Makefile) | Root Makefile, includes all other makefiles. Used to compile and execute the repository files |
| [project-proposal.md](./project-proposal.md) | Original project proposal, 2023-03-02. |

## Instructions for compiling and running

>This repository **DEPENDS** on and expects that it resides in the same parent folder as [step-kth/s3k](https://github.com/kth-step/s3k).

1. Clone both repositories.
2. Make sure that they are in the same parent folder.
3. Cd into s3k-monitor

```
$ git clone https://github.com/zynachs/s3k-monitor.git
$ git clone https://github.com/kth-step/s3k.git
$ ls
s3k  s3k-monitor
$ cd s3k-monitor
```

4. Install prerequisite packages and tools, build from source or install from package manager. The links below are to the sources. the code block is an example of installing with the apt package manager on Ubuntu.
   - [RISC-V GNU Compiler Toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain.git)
   - [QEMU](https://github.com/qemu/qemu)
```
$ sudo apt update
$ sudo apt install -y gcc-riscv64-unknown-elf qemu-system-misc
```

5. Compile kernel, monitor and app with 
```
$ make
```
6. Execute kernel, monitor and app with qemu and launch a GDB live debugging session with 
```
make qemu
```
7. In a separate terminal read output by connection with telnet
```
$ telnet localhost 4321
```

## Memory Layout
