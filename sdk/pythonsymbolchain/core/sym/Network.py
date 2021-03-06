import base64

import sha3

from ..ByteArray import ByteArray
from ..Network import Network as BasicNetwork


class Address(ByteArray):
    """Represents a symbol address."""

    # pylint: disable=too-few-public-methods

    def __init__(self, address):
        """Creates an address from a decoded or encoded address."""
        raw_bytes = address
        if isinstance(raw_bytes, str):
            raw_bytes = base64.b32decode(raw_bytes + 'A')[0:-1]

        super().__init__(24, raw_bytes, Address)

    def __str__(self):
        return base64.b32encode(self.bytes + bytes(0)).decode('utf8')[0:-1]


class Network(BasicNetwork):
    """Represents a symbol network."""

    def address_hasher(self):
        return sha3.sha3_256()

    def create_address(self, address_without_checksum, checksum):
        return Address(address_without_checksum + checksum[0:3])


Network.PUBLIC = Network('public', 0x68)
Network.PRIVATE = Network('private', 0x78)
Network.PUBLIC_TEST = Network('public_test', 0x98)
Network.PRIVATE_TEST = Network('private_test', 0xA8)
Network.NETWORKS = [Network.PUBLIC, Network.PRIVATE, Network.PUBLIC_TEST, Network.PRIVATE_TEST]
