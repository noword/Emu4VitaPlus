from baseio import *
import lz4.block
import json
from zlib import crc32
from cores import CORES
import shutil
import os


KEYS = ('Rom', 'English', "Chinese", "Japanese", "Italian", "French", "Spanish", "Russian")


class NameDB(Base, list):
    def load(self, io):
        size = io.read_uint32()
        str_io = MyBytesIO(io.read(size))
        num = io.read_uint32()
        for _ in range(num):
            item = {}
            item['Crc32'] = io.read_uint32()
            offsets = io.read_uint32(len(KEYS))
            for i, offset in enumerate(offsets):
                str_io.seek(offset, os.SEEK_SET)
                item[KEYS[i]] = str_io.read_cstr()
            self.append(item)

    def save(self, io):
        cache = {}
        str_io = MyBytesIO()
        cache[''] = 0
        str_io.write_cstr('')
        offsets = []
        for item in self:
            offsets.append(item['Crc32'])
            for key in KEYS:
                if item[key] in cache:
                    offset = cache[item[key]]
                else:
                    cache[item[key]] = offset = str_io.tell()
                    str_io.write_cstr(item[key])
                offsets.append(offset)
        str_io.write_padding(4)

        io.write_uint32(str_io.tell())
        io.write(str_io.getvalue())
        io.write_uint32(len(self))
        io.write_uint32(offsets)


class ZNameDB(ZBase, NameDB):
    pass


def str_crc32(s):
    return crc32(str(s).encode('utf-8')) & 0xFFFFFFFF


for json_name, core_name in (
    ('arcade', 'ARC'),
    ('nes', 'NES'),
    ('snes', 'SNES'),
    ('md', 'MD'),
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

    db = ZNameDB()
    for name in names:
        item = {}
        if 'CRC32' in name:
            item['Crc32'] = int(name['CRC32'], 16)
        else:
            item['Crc32'] = str_crc32(name['File'])

        if 'ROM' in name:
            item['Rom'] = name['ROM']
        else:
            item['Rom'] = name['English']

        for key in KEYS[1:]:
            item[key] = name.get(key, '')
        db.append(item)

    db_name = f'{core_name}.zdb'
    db.save(MyFile(db_name, 'wb'))

    shutil.copy(db_name, f'../arch/pkg/data/{core_name}/names.zdb')
    for core in CORES[core_name]:
        dst_path = f'../apps/{core}/pkg/assets'
        if not os.path.exists(dst_path):
            os.mkdir(dst_path)
        shutil.copy(db_name, f'{dst_path}/names.zdb')
