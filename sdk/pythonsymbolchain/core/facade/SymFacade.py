import sha3

from ..CryptoTypes import Hash256
from ..Network import NetworkLocator
from ..sym.KeyPair import KeyPair, Verifier
from ..sym.Network import Network


class SymFacade:
    """Facade used to interact with symbol blockchain."""

    BIP32_COIN_ID = 4343
    BIP32_CURVE_NAME = 'ed25519'

    KeyPair = KeyPair
    Verifier = Verifier

    def __init__(self, network_name):
        """Creates a facade."""
        self.network = NetworkLocator.find_by_name(Network.NETWORKS, network_name)

    @staticmethod
    def hash_buffer(buffer):
        """Hashes a buffer."""
        return Hash256(sha3.sha3_256(buffer).digest())

    @staticmethod
    def bip32_node_to_key_pair(bip32_node):
        """Derives a KeyPair from a BIP32 node."""
        return KeyPair(bip32_node.private_key)
