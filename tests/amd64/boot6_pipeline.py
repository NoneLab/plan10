#!/usr/bin/env python3
"""Unit-test the on-disk 6.out packaging/embedding helpers, not QEMU boot."""
import importlib.util
import pathlib
import struct
import sys
import tempfile

SRC = pathlib.Path(__file__).resolve().parents[2] / 'tools/pc64'


def load(name, filename):
    spec = importlib.util.spec_from_file_location(name, SRC / filename)
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod


elfconv = load('elf6conv', 'elf26out.py')
embed = load('boot6embed', 'genboot6.py')


def fake_elf():
    # Unit-only input: no real boot executable is embedded in the kernel.
    text = b'\x90' * 32
    dat = b'ABCD'
    va = 0x1028
    datava = 0x2000
    header = elfconv.ELF_HDR.pack(
        b'\x7fELF\x02\x01\x01' + bytes(9),
        2, 62, 1, va, 64, 0, 0, 64, 56, 2, 0, 0, 0)
    ph1 = elfconv.PHDR.pack(1, 5, 0x1000, va, va, len(text), len(text), 1)
    ph2 = elfconv.PHDR.pack(1, 6, 0x2000, datava, datava, len(dat), len(dat)+8, 1)
    blob = bytearray(0x2000+len(dat))
    blob[:len(header)] = header
    blob[64:64+len(ph1)] = ph1
    blob[64+len(ph1):64+len(ph1)+len(ph2)] = ph2
    blob[0x1000:0x1000+len(text)] = text
    blob[0x2000:0x2000+len(dat)] = dat
    return bytes(blob)


def main():
    raw, meta = elfconv.make_6out(fake_elf())
    assert meta == (32, 4, 8, 0x1028)
    assert raw[40:] == b'\x90' * 32 + b'ABCD'
    assert embed.validate(raw) == (0x1028, 32, 4, 8)
    with tempfile.TemporaryDirectory() as d:
        path = pathlib.Path(d) / 'boot6.h'
        embed.emit_header(path, raw)
        assert 'static uchar p9_boot6_image[]' in path.read_text()
    for altered in (b'', raw[:15], b'bad!' + raw[4:]):
        try:
            embed.validate(altered)
        except ValueError:
            pass
        else:
            raise AssertionError('invalid 6.out accepted')
    try:
        elfconv.make_6out(fake_elf()[:100])
    except ValueError:
        pass
    else:
        raise AssertionError('truncated ELF accepted')
    print('Plan 9/pc64 boot6 ELF converter and root embedding validator: PASS')
    print('This test does NOT exercise the original boot program or QEMU Ring 3')


if __name__ == '__main__':
    main()