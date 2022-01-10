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

        self.private_key = PrivateKey(hmac_result[0:PrivateKey.SIZE])
        self.chain_code = hmac_result[PrivateKey.SIZE:]

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
    """Factory of BIP32 root nodes """

    def __init__(self, curve_name='ed25519', mnemonic_language='english'):
        """Creates a BIP32 root node factory."""
        self.root_hmac_key = (curve_name + ' seed').encode('utf8')
        self.mnemonic_language = mnemonic_language

    def from_seed(self, seed):
        """Creates a BIP32 root node from a seed."""
        return Bip32Node(self.root_hmac_key, seed)

    def from_mnemonic(self, mnemonic, password):
        """Creates a BIP32 root node from a BIP39 mnemonic and password."""
        return self.from_seed(Mnemonic(self.mnemonic_language).to_seed(mnemonic, password))
