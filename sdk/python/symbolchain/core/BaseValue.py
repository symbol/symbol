from .Ordered import Ordered


class BaseValue(Ordered):
    """Represents a base int."""

    def __init__(self, byte_size, value, tag=None, signed=False):
        """Creates a base value."""
        self.__byte_size = byte_size
        self.value = self._clamp(value, signed)
        self.__tag = (tag, signed)

    def _clamp(self, value, signed):
        bit_size = self.__byte_size * 8
        mask = (1 << bit_size) - 1
        if not signed:
            return value & mask

        sign_bit = 1 << (bit_size - 1)
        return ((value & mask) ^ sign_bit) - sign_bit

    def _cmp(self, other, operation):
        if not isinstance(other, BaseValue):
            return NotImplemented

        # pylint: disable=protected-access
        return operation(self.value, other.value) and self.__tag == other.__tag

    def __eq__(self, other):
        # pylint: disable=protected-access
        return isinstance(other, BaseValue) and self.value == other.value and self.__tag == other.__tag

    def __ne__(self, other):
        return not self == other

    def __hash__(self):
        return hash((self.value, self.__tag))

    def __str__(self):
        if not self.__tag[1] or self.value >= 0:
            return f'0x{self.value:0{self.__byte_size * 2}X}'
        return f'-0x{-self.value:0{self.__byte_size * 2}X}'
