import base64

import sha3

from ..Network import Network as BasicNetwork


class Network(BasicNetwork):
    """Represents a symbol network."""

    def address_hasher(self):
        return sha3.sha3_256()

    def create_encoded_address(self, address_without_checksum, checksum):
        return base64.b32encode(address_without_checksum + checksum[0:3] + bytes(0))[0:-1]


Network.PUBLIC = Network('public', 0x68)
Network.PRIVATE = Network('private', 0x78)
Network.PUBLIC_TEST = Network('public_test', 0x98)
Network.PRIVATE_TEST = Network('private_test', 0xA8)
Network.NETWORKS = [Network.PUBLIC, Network.PRIVATE, Network.PUBLIC_TEST, Network.PRIVATE_TEST]
