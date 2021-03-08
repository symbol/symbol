import sha3

from ..CryptoTypes import Hash256, PrivateKey
from ..Network import NetworkLocator
from ..nis1.KeyPair import KeyPair, Verifier
from ..nis1.Network import Network
from ..nis1.TransactionFactory import TransactionFactory
from ..nis1.TypeParsingRules import TypeParsingRules


class NisFacade:
    """Facade used to interact with nis blockchain."""

    BIP32_COIN_ID = 43
    BIP32_CURVE_NAME = 'ed25519-keccak'

    KeyPair = KeyPair
    Verifier = Verifier

    def __init__(self, network_name, account_descriptor_repository):
        """Creates a facade."""
        self.network = NetworkLocator.find_by_name(Network.NETWORKS, network_name)
        self.account_descriptor_repository = account_descriptor_repository
        self.transaction_factory = TransactionFactory(self.network, TypeParsingRules(self.account_descriptor_repository).as_map())

    @staticmethod
    def hash_buffer(buffer):
        """Hashes a buffer."""
        return Hash256(sha3.keccak_256(buffer).digest())

    @staticmethod
    def bip32_node_to_key_pair(bip32_node):
        """Derives a KeyPair from a BIP32 node."""
        # BIP32 private keys should be used as is, so reverse here to counteract reverse in KeyPair
        return KeyPair(PrivateKey(bip32_node.private_key.bytes[::-1]))
