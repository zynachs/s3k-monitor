#!/usr/bin/env python3
import argparse

if __name__ == "__main__":
    """
    Script to translate a start and end address to a pmpaddr.
    """
    argparse = argparse.ArgumentParser(prog="riscvpmpinv",
                                       description="Translates address range to RISC-V NAPOT pmp.")
    argparse.add_argument('begin', type=lambda x: int(x,0))
    argparse.add_argument('end', type=lambda x: int(x,0))
    args = argparse.parse_args()
    begin = args.begin
    end = args.end
    addr = (begin | (((end - begin) - 1) >> 1)) >> 2
    pmp_begin = ((addr + 1) & addr)  << 2
    pmp_end = (((addr + 1) | addr) + 1) << 2
    if pmp_begin == begin and pmp_end == end:
        print(hex(begin), hex(end), "==>", hex(addr))
    else:
        print("No solution")
