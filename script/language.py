from baseio import *
import lz4.block

LANGS = ('English', "Chinese", "Japanese", "Italian", "French", "Spanish", "Russian")


class Texts(Base, list):
    def load(self, io):
        size = io.read_uint32()
        str_io = MyBytesIO(io.read(size))
        num = io.read_uint32()
        offsets = io.read_uint32(num)
        for offset in offsets:
            str_io.seek(offset, os.SEEK_SET)
            self.append(io.read_cstr())

    def save(self, io):
        str_io = MyBytesIO()
        cache = {}
        offsets = []
        for s in self:
            if s in cache:
                offset = cache[s]
            else:
                offset = cache[s] = str_io.tell()
                str_io.write_cstr(s)
            offsets.append(offset)
        str_io.write_padding(4)

        io.write_uint32(str_io.tell())
        io.write(str_io.getvalue())
        io.write_uint32(len(self))
        io.write_uint32(offsets)


class Languages(Base, dict):
    def load(self, io):
        offsets = io.read_uint32(len(LANGS))
        for i, offset in enumerate(offsets):
            io.seek(offset, os.SEEK_SET)
            self[LANGS[i]] = Texts(io)

    def save(self, io):
        offsets = []
        io.write_uint32([0] * len(self))
        for t in self.values():
            offsets.append(io.tell())
            t.save(io)
            io.write_padding(4)

        io.seek(0, os.SEEK_SET)
        io.write_uint32(offsets)


class ZLanguages(ZBase, Languages):
    pass
