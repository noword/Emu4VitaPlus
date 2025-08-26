import json
import lz4.block
from zlib import crc32
from io import BytesIO
from struct import pack
from cores import CORES
import shutil
import os

LANGS = (
    ('English', 'en'),
    ('Chinese', 'cn'),
)


def str_crc32(s):
    return crc32(str(s).encode('utf-8')) & 0xFFFFFFFF


def gen_db(infos, db_name, core_name):
    map_io = BytesIO()
    name_io = BytesIO(b'\x00')
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
):
    print(json_name)
    names = json.load(open(f'rom_db/{json_name}.names.json', encoding='utf-8'))
    for lang_name, lang_code in LANGS:
        infos = {}
        need_save = False
        for n in names:
            name = n[lang_name]
            if len(name) == 0:
                if lang_name == 'English':
                    continue
                else:
                    name = n['English']
            else:
                need_save = True
            if 'CRC32' in n:
                key = int(n['CRC32'], 16)
            else:
                key = str_crc32(n['File'])
            infos[key] = name
        if not need_save:
            continue
        db_name = f'names.{lang_code}'
        gen_db(infos, db_name, core_name)
        db_name += ".zdb"
        shutil.copy(db_name, f'../arch/pkg/data/{core_name}')
        for core in CORES[core_name]:
            dst_path = f'../apps/{core}/pkg/assets'
            if not os.path.exists(dst_path):
                os.mkdir(dst_path)
            shutil.copy(db_name, f'{dst_path}/{db_name}')
