import base64

import sha3

from ..ByteArray import ByteArray
from ..CryptoTypes import Hash256
from ..Network import Network as BasicNetwork


class Address(ByteArray):
    """Represents a symbol address."""

    SIZE = 24

    def __init__(self, address):
        """Creates an address from a decoded or encoded address."""
        raw_bytes = address
        if isinstance(address, str):
            raw_bytes = base64.b32decode(address + 'A')[0:-1]
        elif isinstance(address, Address):
            raw_bytes = address.bytes

        super().__init__(self.SIZE, raw_bytes, Address)

    def __str__(self):
        return base64.b32encode(self.bytes + bytes(0)).decode('utf8')[0:-1]


class Network(BasicNetwork):
    """Represents a symbol network."""

    def __init__(self, name, identifier, generation_hash_seed=None):
        """Creates a new network with the specified name, identifier byte and generation hash seed."""
        super().__init__(name, identifier)
        self.generation_hash_seed = generation_hash_seed

    def address_hasher(self):
        return sha3.sha3_256()

    def create_address(self, address_without_checksum, checksum):
        return Address(address_without_checksum + checksum[0:3])


Network.PUBLIC = Network('public', 0x68, Hash256('57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6'))
Network.PRIVATE = Network('private', 0x78)
Network.PUBLIC_TEST = Network('public_test', 0x98, Hash256('3B5E1FA6445653C971A50687E75E6D09FB30481055E3990C84B25E9222DC1155'))
Network.PRIVATE_TEST = Network('private_test', 0xA8)
Network.NETWORKS = [Network.PUBLIC, Network.PRIVATE, Network.PUBLIC_TEST, Network.PRIVATE_TEST]
