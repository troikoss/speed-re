"""Analysis-only loader for fixed-address i386 GCC objects in Unicorn."""
import struct
import subprocess


def link_native(source, build, root, bindings, entry, extra_objects=(), gc_sections=False):
    build.mkdir(parents=True, exist_ok=True)
    sources = source if isinstance(source, (list, tuple)) else [source]
    objects = []
    exe = build / 'native.elf'
    for i, item in enumerate(sources):
        obj = build / ('native.o' if len(sources) == 1 else 'native-' + str(i) + '.o')
        subprocess.run(['g++', '-std=c++11', '-m32', '-O1', '-fno-pie', '-mfpmath=387', '-fno-stack-protector',
                        '-fno-strict-aliasing', '-fno-fast-math', '-ffp-contract=off',
                        '-include', str(root / 'src/Platform/Native/NativeCompat.h'),
                        '-Isrc/Speed/Indep/Src/World', '-c', str(item), '-o', str(obj)],
                       cwd=root, check=True)
        objects.append(str(obj))
    subprocess.run(['ld', '-m', 'elf_i386', '-Ttext=0x2400000', '--entry=' + entry] +
                   (['--gc-sections'] if gc_sections else []) +
                   ['--defsym=' + k + '=' + hex(v) for k, v in bindings.items()] +
                   objects + [str(obj) for obj in extra_objects] + ['-o', str(exe)], check=True)
    raw = exe.read_bytes()
    phoff = struct.unpack_from('<I', raw, 28)[0]
    phsize, phcount = struct.unpack_from('<HH', raw, 42)
    blocks = []
    for i in range(phcount):
        kind, off, addr, _, filesz, memsz, _, _ = struct.unpack_from('<8I', raw, phoff + i * phsize)
        if kind == 1 and memsz:
            blocks.append((addr, raw[off:off + filesz] + bytes(memsz - filesz)))
    low = min(a for a, _ in blocks) & ~4095
    high = (max(a + len(b) for a, b in blocks) + 4095) & ~4095
    exports = {}
    for line in subprocess.check_output(['nm', '-g', str(exe)], text=True).splitlines():
        parts = line.split()
        if len(parts) == 3:
            exports[parts[2]] = int(parts[0], 16)
    return dict(address=low, allocation=high - low, blocks=blocks, exports=exports)
