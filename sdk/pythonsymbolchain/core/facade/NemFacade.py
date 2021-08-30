import sha3

from ..CryptoTypes import Hash256, PrivateKey, PublicKey
from ..nem.KeyPair import KeyPair, Verifier
from ..nem.Network import Address, Network
from ..nem.TransactionFactory import TransactionFactory
from ..Network import NetworkLocator


class NemFacade:
    """Facade used to interact with nem blockchain."""

    BIP32_COIN_ID = 43
    BIP32_CURVE_NAME = 'ed25519-keccak'

    Address = Address
    KeyPair = KeyPair
    Verifier = Verifier

    def __init__(self, nem_network_name, account_descriptor_repository=None):
        """Creates a nem facade."""
        self.network = NetworkLocator.find_by_name(Network.NETWORKS, nem_network_name)
        self.account_descriptor_repository = account_descriptor_repository
        self.transaction_factory = self._create_nem_transaction_factory()

    def _create_nem_transaction_factory(self):
        type_parsing_rules = None
        if self.account_descriptor_repository:
            type_parsing_rules = self.account_descriptor_repository.to_type_parsing_rules_map(self._create_nem_type_to_property_mapping())

        return TransactionFactory(self.network, type_parsing_rules)

    @staticmethod
    def _create_nem_type_to_property_mapping():
        return {
            Address: 'address',
            PublicKey: 'public_key'
        }

    @staticmethod
    def hash_transaction(transaction):
        """Hashes a nem transaction."""
        return Hash256(sha3.keccak_256(transaction.serialize()).digest())

    @staticmethod
    def sign_transaction(key_pair, transaction):
        """Signs a nem transaction."""
        return key_pair.sign(transaction.serialize())

    @staticmethod
    def verify_transaction(transaction, signature):
        """Verifies a nem transaction."""
        return Verifier(transaction.signer_public_key).verify(transaction.serialize(), signature)

    @staticmethod
    def bip32_node_to_key_pair(bip32_node):
        """Derives a nem KeyPair from a BIP32 node."""
        # BIP32 private keys should be used as is, so reverse here to counteract reverse in KeyPair
        return KeyPair(PrivateKey(bip32_node.private_key.bytes[::-1]))
