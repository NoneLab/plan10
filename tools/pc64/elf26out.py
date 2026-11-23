#!/usr/bin/env python3
"""Convert one *fully linked* native AMD64 boot ELF to original Plan 9 S_MAGIC 6.out.

The input must have a RX PT_LOAD beginning at 0x1028 and a separate RW
PT_LOAD beginning at the next page. This script deliberately rejects other
layouts instead of silently manufacturing a non-bootable image.
"""
import argparse
import pathlib
import struct

S_MAGIC = 0x8000 | (4 * 26 * 26 + 7)
TEXT_BASE = 0x1000 + 40
PAGE = 4096
ELF_HDR = struct.Struct('<16sHHIQQQIHHHHHH')
PHDR = struct.Struct('<IIQQQQQQ')


def roundup(value, align=PAGE):
    return (value + align - 1) & ~(align - 1)


def make_6out(data):
    if len(data) < ELF_HDR.size:
        raise ValueError('truncated ELF header')
    
    e = ELF_HDR.unpack_from(data)
    ident, typ, mach, version, entry, phoff, _shoff, _flags, ehsz, phentsz, phnum, *_ = e
    
    if ident[:7] != b'\x7fELF\x02\x01\x01' or typ != 2 or mach != 62 or version != 1:
        raise ValueError('expected static x86_64 ELF64 ET_EXEC, little-endian')
    
    if phentsz != PHDR.size or ehsz != ELF_HDR.size or phnum < 1 or phnum > 32:
        raise ValueError('unexpected ELF program-header layout')
    
    if phoff > len(data) or phoff + phnum * phentsz > len(data):
        raise ValueError('truncated program-header table')
    
    loads = []
    for i in range(phnum):
        p = PHDR.unpack_from(data, phoff + i * phentsz)
        typ, flags, off, va, _pa, sz, memsz, align = p
        if typ != 1:
            continue
        if sz > memsz or off > len(data) or off + sz > len(data):
            raise ValueError('invalid PT_LOAD file/memory bounds')
        if va > 0xffffffffffffffff - memsz:
            raise ValueError('PT_LOAD address overflow')
        loads.append((flags, va, sz, memsz, data[off:off+sz]))
    
    if len(loads) != 2:
        raise ValueError(f'expected exactly 2 PT_LOADs (RX and RW), got {len(loads)}')
    
    text, rw = loads
    if text[0] != 5 or rw[0] != 6:
        raise ValueError('expected RX text and RW data PT_LOADs')
    
    if text[1] != TEXT_BASE or not text[2] or text[2] != text[3]:
        raise ValueError('text PT_LOAD must start at 0x1028 and have no BSS')
    
    data_base = roundup(TEXT_BASE + text[2])
    if rw[1] != data_base:
        raise ValueError(f'data PT_LOAD begins {rw[1]:#x}, expected {data_base:#x}')
    
    if not TEXT_BASE <= entry < TEXT_BASE + text[2]:
        raise ValueError(f'entry {entry:#x} is outside user text segment')
    
    txsz, dsz, bsz = text[2], rw[2], rw[3] - rw[2]
    if max(txsz, dsz, bsz) > 0xffffffff:
        raise ValueError('6.out fields exceed 32-bit sizes')
    
    hdr = struct.pack('>8IQ', S_MAGIC, txsz, dsz, bsz, 0, entry & 0xffffffff, 0, 0, entry)
    assert len(hdr) == 40
    return hdr + text[4] + rw[4], (txsz, dsz, bsz, entry)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('elf', type=pathlib.Path)
    parser.add_argument('output', type=pathlib.Path)
    args = parser.parse_args()
    data, meta = make_6out(args.elf.read_bytes())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(data)
    t, d, b, e = meta
    print(f'Plan 9/amd64 native 6.out: {len(data)} bytes text={t} data={d} bss={b} entry={e:#x}')


if __name__ == '__main__':
    main()