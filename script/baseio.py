#! /usr/bin/env python
# coding=utf-8
from struct import unpack, pack, calcsize
from io import BytesIO, FileIO
from collections.abc import Iterable
import os
import lz4.block

DEFAULT_ENDIAN = '<'


class BaseIO:
    __endian = DEFAULT_ENDIAN
    __endian_stack = []

    def set_bigendian(self):
        self.__endian = '>'

    def set_littleendian(self):
        self.__endian = '<'

    def push_endian(self):
        self.__endian_stack.append(self.__endian)

    def pop_endian(self):
        self.__endian = self.__endian_stack.pop()

    def _read(self, format, num):
        if num == -1:
            _format = f'{self.__endian}{format}'
        elif num == 0:
            return []
        else:
            _format = f'{self.__endian}{num}{format}'
        value = list(unpack(_format, self.read(calcsize(_format))))
        return value[0] if num == -1 else value

    def _write(self, format, value):
        if isinstance(value, Iterable):
            self.write(pack(f'{self.__endian}{len(value)}{format}', *value))
        else:
            self.write(pack(f'{self.__endian}{format}', value))

    def read_uint8(self, num=-1):
        return self._read('B', num)

    def read_int8(self, num=-1):
        return self._read('b', num)

    def read_uint16(self, num=-1):
        return self._read('H', num)

    def read_int16(self, num=-1):
        return self._read('h', num)

    def read_uint32(self, num=-1):
        return self._read('I', num)

    def read_int32(self, num=-1):
        return self._read('i', num)

    def read_float(self, num=-1):
        return self._read('f', num)

    def read_int64(self, num=-1):
        return self._read('q', num)

    def read_uint64(self, num=-1):
        return self._read('Q', num)

    def read_cstr(self, encoding='utf-8'):
        value = b''
        tmp = self.read(1)
        while tmp != b'\x00' and len(tmp) > 0:
            value += tmp
            tmp = self.read(1)
        return value.decode(encoding)

    def read_str(self, encoding='utf-8'):
        size = self.read_uint32()
        value = self.read(size)
        self.seek_align(4)
        return value.decode(encoding)

    def write_uint8(self, value):
        self._write('B', value)

    def write_int8(self, value):
        self._write('b', value)

    def write_uint16(self, value):
        self._write('H', value)

    def write_int16(self, value):
        self._write('h', value)

    def write_uint32(self, value):
        self._write('I', value)

    def write_int32(self, value):
        self._write('i', value)

    def write_int64(self, value):
        self._write('q', value)

    def write_uint64(self, value):
        self._write('Q', value)

    def write_float(self, value):
        self._write('f', value)

    def write_cstr(self, value, encoding='utf-8'):
        self.write(value.encode(encoding) + b'\x00')

    def write_str(self, value, encoding='utf-8'):
        value = value.encode(encoding)
        self.write_uint32(len(value))
        self.write(value)
        self.seek_align(4)

    def read_relative_offset(self):
        offset = self.read_uint32()
        if offset == 0:
            return None
        else:
            return self.tell() - 4 + offset

    def write_relative_offset(self, offset):
        if offset is None:
            self.write_uint32(0)
        else:
            offset -= self.tell()
            self.write_uint32(offset)

    def seek_align(self, alignment):
        alignment -= 1
        self.seek((self.tell() + alignment) & (~alignment), os.SEEK_SET)

    def write_padding(self, align, value=b'\x00'):
        align -= 1
        offset = self.tell()
        size = ((offset + align) & ~align) - offset
        self.write(value * size)


class MyFile(FileIO, BaseIO):
    pass


class MyBytesIO(BytesIO, BaseIO):
    pass


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


class ZBase:
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
