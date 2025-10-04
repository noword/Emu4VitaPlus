import xml.etree.ElementTree as ET
from pathlib import Path
from zlib import crc32
from struct import pack, unpack
import lz4.block
from arc_dat import *


def Grab(path, pattern):
    data_io = MyBytesIO()
    name_offsets = {}
    names = set()
    roms = {}

    for name in Path(path).glob(pattern):
        tree = ET.parse(name)
        root = tree.getroot()
        for game in root.iter('game'):
            name = game.attrib['name']
            names.add(crc32(name.encode('utf-8')) & 0xFFFFFFFF)
            if name not in name_offsets:
                name_offsets[name] = data_io.tell()
                data_io.write(name.encode('utf-8'))
                data_io.write(b'\x00')

            for rom in game.iter('rom'):
                crc = rom.get('crc')
                if crc:
                    if crc in roms:
                        roms[crc].add(name_offsets[name])
                    else:
                        roms[crc] = {name_offsets[name]}

    data_io.write_padding(4)
    arcdat = ZArcDat()
    arcdat.name_buf = data_io.getvalue()
    arcdat.name_crc = names
    arcdat.roms = []
    for crc, offsets in roms.items():
        rom = Rom()
        rom.crc32 = int(crc, 16)
        rom.name_offsets = offsets
        arcdat.roms.append(rom)

    return arcdat


Grab('../cores/libretro-fbneo/dats/', '*.dat').save(MyFile('fba_dat.zbin', 'wb'))
Grab('../cores/mame2003-plus-libretro/metadata', '*.xml').save(MyFile('mame_dat.zbin', 'wb'))
