import base64

import sha3

from ..Network import Network as BasicNetwork


class Network(BasicNetwork):
    """Represents a nis network."""

    def address_hasher(self):
        return sha3.keccak_256()

    def create_encoded_address(self, address_without_checksum, checksum):
        return base64.b32encode(address_without_checksum + checksum)


Network.MAINNET = Network('mainnet', 0x68)
Network.TESTNET = Network('testnet', 0x98)
Network.NETWORKS = [Network.MAINNET, Network.TESTNET]
