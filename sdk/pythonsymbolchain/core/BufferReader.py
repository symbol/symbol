from binascii import hexlify


class BufferReader:
    """Reads data from an in memory buffer."""

    def __init__(self, buffer, byte_order='little'):
        """Creates a reader with specified byte order."""
        self.buffer = buffer
        self.byte_order = byte_order
        self.offset = 0

    @property
    def eof(self):
        """Returns true if the reader is at eof."""
        return self.offset == len(self.buffer)

    def read_int(self, count):
        """Reads an integer."""
        return int.from_bytes(self.read_bytes(count), self.byte_order)

    def read_string(self, count):
        """Reads a string."""
        return self.read_bytes(count).decode('utf8')

    def read_hex_string(self, count):
        """Reads a hex string."""
        return hexlify(self.read_bytes(count)).decode('utf8').upper()

    def read_bytes(self, count):
        """Reads bytes."""
        buffer = self.buffer[self.offset:self.offset + count]
        self.offset += count
        return buffer
