"""COFF linkage for recovered response dispatch and its bounded queue helpers."""
import os
import subprocess
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT

# Original call boundaries not recovered by the response component itself.
BOUNDARIES = {
    'Post': (0x51aa20, 5), 'FindObjectByGuid': (0x517fe0, 1),
    'SetScript': (0x5178b0, 1), 'SetFocus': (0x52b3a0, 3),
    'Call51AF50': (0x51af50, 3), 'Call51AB20': (0x51ab20, 2),
    'Call51AC10': (0x51ac10, 3), 'Call51ADB0': (0x51adb0, 1),
    'Call51B070': (0x51b070, 1),
}


def link_response(compiler, source, state_source, build):
    objects = []
    for path in (source, state_source):
        obj = build / (path.stem + '.obj')
        command = ([str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe'] if compiler == 'msvc' else
                   ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32', '/clang:-fno-strict-aliasing'])
        subprocess.run(command + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)),
                                   '/Isrc/Speed/Indep/Src/Frontend', os.path.relpath(path, ROOT)], cwd=ROOT, check=True)
        objects.append(obj)
    external = {}
    for obj in objects:
        for symbol in read_object(obj)['symbols'].values():
            if symbol['section'] == 0:
                for name, (address, _) in BOUNDARIES.items():
                    if symbol['name'].startswith('?' + name + '@'):
                        external[symbol['name']] = address
    return link_objects(objects, external)
