from binascii import hexlify, unhexlify


class ByteArray:
    """Represents a fixed size byte array."""

    def __init__(self, fixed_size, array_input, tag=None):
        """Creates a byte array from bytes or hex string."""
        raw_bytes = array_input
        if isinstance(raw_bytes, str):
            raw_bytes = unhexlify(raw_bytes)

        if fixed_size != len(raw_bytes):
            raise ValueError('bytes was size {} but must be {}'.format(len(raw_bytes), fixed_size))

        self.bytes = raw_bytes
        self.__tag = tag

    def __eq__(self, other):
        # pylint: disable=protected-access
        return isinstance(other, ByteArray) and self.bytes == other.bytes and self.__tag == other.__tag

    def __str__(self):
        return hexlify(self.bytes).decode('utf8').upper()
