from baseio import *
import lz4.block


class Rom(Base, list):
    def load(self, io):
        num, self.crc32 = io.read_uint32(2)
        self.name_offsets = io.read_uint32(num)

    def save(self, io):
        io.write_uint32((len(self.name_offsets), self.crc32))
        io.write_uint32(self.name_offsets)


class ArcDat(Base):
    def load(self, io):
        size = io.read_uint32()
        self.name_buf = io.read(size)
        num = io.read_uint32()
        self.name_crc = io.read_uint32(num)
        num = io.read_int32()
        self.roms = [Rom(io) for i in range(num)]

    def save(self, io):
        io.write_uint32(len(self.name_buf))
        io.write(self.name_buf)
        io.write_uint32(len(self.name_crc))
        io.write_uint32(self.name_crc)
        io.write_uint32(len(self.roms))
        [rom.save(io) for rom in self.roms]


class ZArcDat(ZBase, ArcDat):
    pass
