import json
import lz4.block
from zlib import crc32
from io import BytesIO
from struct import pack
from cores import CORES
import shutil
import os


LANGS = ('English', "Chinese", "Japanese", "Italian", "French", "Spanish", "Russian")


def str_crc32(s):
    return crc32(str(s).encode('utf-8')) & 0xFFFFFFFF


def gen_db(infos, db_name, core_name):
    map_io = BytesIO()
    name_io = BytesIO()
    name_io.write(b'\x00')
    map_io.write(pack('I', len(infos)))
    for key, name in infos.items():
        map_io.write(pack('II', key, name_io.tell()))
        name_io.write(name.encode('utf-8') + b'\x00')

    buf = map_io.getvalue() + pack('I', name_io.tell()) + name_io.getvalue()
    zbuf = lz4.block.compress(buf, mode='high_compression', store_size=False, compression=12)

    open(f'{db_name}.{core_name}.bin', 'wb').write(buf)

    with open(db_name + '.zdb', 'wb') as fp:
        fp.write(pack('II', len(buf), len(zbuf)))
        fp.write(zbuf)


for json_name, core_name in (
    ('arcade', 'ARC'),
    ('nes', 'NES'),
    ('snes', 'SNES'),
    ('gba', 'GBA'),
    ('gbc', 'GBC'),
    ('msx', 'MSX'),
    ('ngp', 'NGP'),
    ('wsc', 'WSC'),
    ('pce', 'PCE'),
    ('atari2600', 'ATARI2600'),
    ('atari5200', 'ATARI5200'),
    ('atari7800', 'ATARI7800'),
):
    print(json_name)
    names = json.load(open(f'rom_db/{json_name}.names.json', encoding='utf-8'))

    name_io = BytesIO()
    name_io.write(b'\x00')
    name_dict = {'': 0}
    for name in names:
        rom = name.get('ROM', '')
        if rom not in name_dict:
            name_dict[rom] = name_io.tell()
            name_io.write(rom.encode('utf-8') + b'\x00')

        for lang in LANGS:
            n = name.get(lang, '')
            if n not in name_dict:
                name_dict[n] = name_io.tell()
                name_io.write(n.encode('utf-8') + b'\x00')

    map_io = BytesIO()
    map_io.write(pack('I', len(names)))
    for name in names:
        if 'CRC32' in name:
            crc = int(name['CRC32'], 16)
        else:
            crc = str_crc32(name['File'])

        if 'ROM' in name:
            rom = name['ROM']
        else:
            rom = name['English']

        d = [crc, name_dict[rom]]
        for lang in LANGS:
            d.append(name_dict[name.get(lang, '')])
        map_io.write(pack(f'{len(d)}I', *d))

    buf = map_io.getvalue() + pack('I', name_io.tell()) + name_io.getvalue()
    zbuf = lz4.block.compress(buf, mode='high_compression', store_size=False, compression=12)
    open(f'{core_name}.bin', 'wb').write(buf)
    db_name = f'{core_name}.zdb'
    with open(db_name, 'wb') as fp:
        fp.write(pack('II', len(buf), len(zbuf)))
        fp.write(zbuf)

    shutil.copy(db_name, f'../arch/pkg/data/{core_name}/names.zdb')
    for core in CORES[core_name]:
        dst_path = f'../apps/{core}/pkg/assets'
        if not os.path.exists(dst_path):
            os.mkdir(dst_path)
        shutil.copy(db_name, f'{dst_path}/names.zdb')
