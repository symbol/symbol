import sha3

from ..Network import NetworkLocator
from ..sym.KeyPair import KeyPair, Verifier
from ..sym.Network import Network


class SymFacade:
    """Facade used to interact with symbol blockchain."""

    # pylint: disable=too-few-public-methods

    Hasher = sha3.sha3_256
    KeyPair = KeyPair
    Verifier = Verifier

    def __init__(self, network_name):
        """Creates a facade."""
        NetworkLocator.find_by_name(Network.NETWORKS, network_name)
