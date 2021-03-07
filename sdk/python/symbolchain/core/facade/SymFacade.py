import sha3

from ..CryptoTypes import Hash256
from ..Network import NetworkLocator
from ..sym.KeyPair import KeyPair, Verifier
from ..sym.Network import Network


class SymFacade:
    """Facade used to interact with symbol blockchain."""

    # pylint: disable=too-few-public-methods

    KeyPair = KeyPair
    Verifier = Verifier

    def __init__(self, network_name):
        """Creates a facade."""
        NetworkLocator.find_by_name(Network.NETWORKS, network_name)

    @staticmethod
    def hash_buffer(buffer):
        """Hashes a buffer."""
        return Hash256(sha3.sha3_256(buffer).digest())
