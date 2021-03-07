import sha3

from ..Network import NetworkLocator
from ..nis1.KeyPair import KeyPair, Verifier
from ..nis1.Network import Network
from ..nis1.TransactionFactory import TransactionFactory
from ..nis1.TypeParsingRules import TypeParsingRules


class NisFacade:
    """Facade used to interact with nis blockchain."""

    # pylint: disable=too-few-public-methods

    Hasher = sha3.keccak_256
    KeyPair = KeyPair
    Verifier = Verifier

    def __init__(self, network_name, account_descriptor_repository):
        """Creates a facade."""
        network = NetworkLocator.find_by_name(Network.NETWORKS, network_name)
        self.account_descriptor_repository = account_descriptor_repository
        self.transaction_factory = TransactionFactory(network, TypeParsingRules(self.account_descriptor_repository).as_map())
