from binascii import unhexlify


class BufferWriter:
    """Writes data to an in memory buffer."""

    def __init__(self, byte_order='little'):
        """Creates a writer with specified byte order."""
        self.byte_order = byte_order
        self.buffer = bytes()

    def write_int(self, value, count):
        """Writes an integer."""
        self.buffer += value.to_bytes(count, self.byte_order)

    def write_string(self, value):
        """Writes a string."""
        self.buffer += value.encode('utf8')

    def write_hex_string(self, value):
        """Writes a hex string."""
        self.buffer += unhexlify(value)

    def write_bytes(self, value):
        """Writes bytes."""
        self.buffer += value
