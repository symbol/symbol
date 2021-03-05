import hashlib
import hmac

from mnemonic import Mnemonic

from .BufferWriter import BufferWriter
from .CryptoTypes import PrivateKey


class Bip32Node():
    """Representation of a BIP32 node."""

    def __init__(self, key, data):
        """Creates a BIP32 node around a key and data."""
        hmac_result = hmac.new(key, data, hashlib.sha512).digest()

        self.private_key = PrivateKey(hmac_result[0:32])
        self.chain_code = hmac_result[32:]

    def derive_one(self, identifier):
        """Derives a direct child node with specified identifier."""
        hmac_data_writer = BufferWriter('big')
        hmac_data_writer.write_int(0, 1)
        hmac_data_writer.write_bytes(self.private_key.bytes)
        hmac_data_writer.write_int(0x80000000 | identifier, 4)
        return Bip32Node(self.chain_code, hmac_data_writer.buffer)

    def derive_path(self, path):
        """Derives a descendent node with specified path."""
        next_node = self
        for identifier in path:
            next_node = next_node.derive_one(identifier)

        return next_node


class Bip32():
    """Factory of root BIP32 nodes """
    @staticmethod
    def from_seed(seed):
        """Creates a root BIP32 node from a seed."""
        root_node_prefix = b'ed25519 seed'
        return Bip32Node(root_node_prefix, seed)

    @staticmethod
    def from_mnemonic(mnemonic, password):
        """Creates a root BIP32 node from a BIP39 mnemonic and password."""
        return Bip32.from_seed(Mnemonic('english').to_seed(mnemonic, password))
