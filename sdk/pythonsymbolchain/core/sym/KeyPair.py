from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ed25519

from ..CryptoTypes import PublicKey, Signature


class KeyPair:
    """Represents an ED25519 private and public key."""

    def __init__(self, private_key):
        """Creates a key pair from a private key."""
        self.__sk = ed25519.Ed25519PrivateKey.from_private_bytes(private_key.bytes)

    @property
    def public_key(self):
        """Gets public key."""
        return PublicKey(self.__sk.public_key().public_bytes(serialization.Encoding.Raw, serialization.PublicFormat.Raw))

    def sign(self, message):
        """Signs a message with the private key."""
        return Signature(self.__sk.sign(message))


class Verifier:
    """Verifies signatures signed by a single key pair."""

    # pylint: disable=too-few-public-methods

    def __init__(self, public_key):
        """Creates a verifier from a public key."""
        self.__pk = ed25519.Ed25519PublicKey.from_public_bytes(public_key.bytes)

    def verify(self, message, signature):
        """Verifies a message signature."""
        try:
            self.__pk.verify(signature.bytes, message)
            return True
        except InvalidSignature:
            return False
