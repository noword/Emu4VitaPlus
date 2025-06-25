from baseio import *
import lz4.block


class Base:
    def __init__(self, io=None):
        if io:
            self.load(io)

    def load(self, io):
        raise NotImplementedError

    def save(self, io):
        raise NotImplementedError

    def getvalue(self):
        io = MyBytesIO()
        self.save(io)
        return io.getvalue()


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


class ZArcDat(ArcDat):
    def load(self, io):
        size, zsize = io.read_uint32(2)
        zbuf = io.read()
        buf = lz4.block.decompress(zbuf, uncompressed_size=size)
        super().load(MyBytesIO(buf))

    def save(self, io):
        _io = MyBytesIO()
        super().save(_io)
        buf = _io.getvalue()
        zbuf = lz4.block.compress(buf, mode='high_compression', store_size=False, compression=12)
        io.write_uint32((len(buf), len(zbuf)))
        io.write(zbuf)


za = ZArcDat(MyFile('arcade_dat.zbin', 'rb'))
za.save(MyFile('1.bin', 'wb'))
