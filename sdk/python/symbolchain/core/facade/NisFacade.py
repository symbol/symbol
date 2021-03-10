import sha3

from ..CryptoTypes import Hash256, PrivateKey, PublicKey
from ..Network import NetworkLocator
from ..nis1.KeyPair import KeyPair, Verifier
from ..nis1.Network import Address, Network
from ..nis1.TransactionFactory import TransactionFactory


class NisFacade:
    """Facade used to interact with nis blockchain."""

    BIP32_COIN_ID = 43
    BIP32_CURVE_NAME = 'ed25519-keccak'

    KeyPair = KeyPair
    Verifier = Verifier

    def __init__(self, network_name, account_descriptor_repository=None):
        """Creates a nis facade."""
        self.network = NetworkLocator.find_by_name(Network.NETWORKS, network_name)
        self.account_descriptor_repository = account_descriptor_repository

        type_parsing_rules = None
        if self.account_descriptor_repository:
            type_parsing_rules = self.account_descriptor_repository.to_type_parsing_rules_map({
                Address: 'address',
                PublicKey: 'public_key'
            })

        self.transaction_factory = TransactionFactory(self.network, type_parsing_rules)

    @staticmethod
    def hash_transaction_buffer(transaction_buffer):
        """Hashes a nis transaction buffer."""
        return Hash256(sha3.keccak_256(transaction_buffer).digest())

    @staticmethod
    def sign_transaction_buffer(key_pair, transaction_buffer):
        """Signs a nis transaction buffer."""
        return key_pair.sign(transaction_buffer)

    @staticmethod
    def bip32_node_to_key_pair(bip32_node):
        """Derives a nis KeyPair from a BIP32 node."""
        # BIP32 private keys should be used as is, so reverse here to counteract reverse in KeyPair
        return KeyPair(PrivateKey(bip32_node.private_key.bytes[::-1]))
