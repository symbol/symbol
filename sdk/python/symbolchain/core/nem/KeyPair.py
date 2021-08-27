import sha3

from ..CryptoTypes import PrivateKey, PublicKey, Signature
from .external import ed25519


class KeyPair:
    """Represents an ED25519 private and public key."""

    def __init__(self, private_key):
        """Creates a key pair from a private key."""
        self._sk = private_key.bytes[::-1]
        self._pk = ed25519.publickey_hash_unsafe(self._sk, sha3.keccak_512)

    @property
    def public_key(self):
        """Gets the public key."""
        return PublicKey(self._pk)

    @property
    def private_key(self):
        """Gets the private key."""
        return PrivateKey(self._sk[::-1])

    def sign(self, message):
        """Signs a message with the private key."""
        return Signature(ed25519.signature_hash_unsafe(message, self._sk, self._pk, sha3.keccak_512))


class Verifier:
    """Verifies signatures signed by a single key pair."""

    def __init__(self, public_key):
        """Creates a verifier from a public key."""
        self._pk = public_key.bytes

    def verify(self, message, signature):
        """Verifies a message signature."""
        try:
            ed25519.checkvalid_hash(signature.bytes, message, self._pk, sha3.keccak_512)
            return True
        except (ValueError, ed25519.SignatureMismatch):
            return False
