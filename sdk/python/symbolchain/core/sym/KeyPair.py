from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ed25519

from ..CryptoTypes import PrivateKey, PublicKey, Signature


class KeyPair:
    """Represents an ED25519 private and public key."""

    def __init__(self, private_key):
        """Creates a key pair from a private key."""
        self._sk = ed25519.Ed25519PrivateKey.from_private_bytes(private_key.bytes)

    @property
    def public_key(self):
        """Gets the public key."""
        return PublicKey(self._sk.public_key().public_bytes(serialization.Encoding.Raw, serialization.PublicFormat.Raw))

    @property
    def private_key(self):
        """Gets the private key."""
        return PrivateKey(self._sk.private_bytes(
                encoding=serialization.Encoding.Raw,
                format=serialization.PrivateFormat.Raw,
                encryption_algorithm=serialization.NoEncryption()))

    def sign(self, message):
        """Signs a message with the private key."""
        return Signature(self._sk.sign(message))


class Verifier:
    """Verifies signatures signed by a single key pair."""

    def __init__(self, public_key):
        """Creates a verifier from a public key."""
        self._pk = ed25519.Ed25519PublicKey.from_public_bytes(public_key.bytes)

    def verify(self, message, signature):
        """Verifies a message signature."""
        try:
            self._pk.verify(signature.bytes, message)
            return True
        except InvalidSignature:
            return False
