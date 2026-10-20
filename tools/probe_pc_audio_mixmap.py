#!/usr/bin/env python3
"""Inspect UG2 MIXMAPS .mxb master-channel records without exporting assets.

Provenance: 0x4837c0 selects a group by the top-level offset table;
0x480320 selects its +0x10 section; 0x479030 advances the channel prefix;
0x480420 reads +0x14 maps and their low-nibble lengths. This is a bounded
analysis reader, not recovered runtime parsing or mixer implementation.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct


def inspect(path):
    body = path.read_bytes()

    def word(offset):
        if offset < 0 or offset + 4 > len(body):
            raise ValueError('%s: word outside file at %#x' % (path.name, offset))
        return struct.unpack_from('<I', body, offset)[0]

    groups = []
    for index in range(word(4)):
        base = word(word(8) + index * 4)
        section_offset, map_offset = word(base + 0x10), word(base + 0x14)
        summary = dict(index=index, offset=hex(base), kind=word(base), master_channels=0, engine=[])
        if section_offset != 0xffffffff:
            section = base + section_offset
            count = word(section)
            summary['master_channels'] = count
            summary['output_groups'] = word(section + 4)
            cursor, mapping = section + 16, base + map_offset
            for channel in range(count):
                key = word(cursor)
                inputs = (key >> 16) & 0xff
                prefix = 2 if key & 0x10000000 else 3
                output = word(cursor + 8) if prefix == 3 else None
                input_words = [word(cursor + (prefix + j) * 4) for j in range(inputs)]
                map_count = word(mapping) & 0xf
                map_words = [word(mapping + 4 + j * 4) for j in range(map_count)]
                # Engine descriptor 0x7ed454 is kind 1/component 0; the
                # resolver's playback tag is 0x40000000 (0x468f70).
                if output == 0x40010000:
                    summary['engine'].append(dict(channel=channel, record_offset=hex(cursor),
                        key=hex(key), inputs=[hex(value) for value in input_words],
                        maps=[dict(word=hex(value), pointer_slot=(value >> 16) & 0xf,
                                   output_slot=(value >> 24) & 0xf,
                                   spatial_slot=(value >> 20) & 0xf,
                                   bias=struct.unpack('<h', struct.pack('<H', value & 0xffff))[0])
                              for value in map_words]))
                cursor += (prefix + inputs) * 4
                mapping += (map_count + 1) * 4
        groups.append(summary)
    return dict(file=path.name, size=len(body), sha256=hashlib.sha256(body).hexdigest(), groups=groups)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', type=Path, default=os.environ.get('UG2_DIR'))
    args = parser.parse_args()
    if args.game is None:
        parser.error('pass --game or set UG2_DIR')
    paths = sorted((args.game / 'SOUND/MIXMAPS').glob('*.mxb'))
    if not paths:
        parser.error('no SOUND/MIXMAPS/*.mxb files found')
    print(json.dumps([inspect(path) for path in paths], indent=2))


if __name__ == '__main__':
    main()
