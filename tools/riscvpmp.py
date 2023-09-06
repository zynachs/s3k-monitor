#!/usr/bin/env python3
import argparse

if __name__ == "__main__":
    """
    Script to translate a pmpaddr to start and end addresses.
    """
    argparse = argparse.ArgumentParser(prog="riscvpmp",
                                       description="Translates RISC-V NAPOT PMP address to concrete beginning and ends.")
    argparse.add_argument('pmpaddr', type=lambda x: int(x,0))
    args = argparse.parse_args()
    pmpaddr = args.pmpaddr
    begin = ((pmpaddr + 1) & pmpaddr) << 2
    end = (((pmpaddr + 1) | pmpaddr) + 1) << 2
    print(hex(pmpaddr), "==>", hex(begin), hex(end))
