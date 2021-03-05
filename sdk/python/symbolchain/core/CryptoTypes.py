import secrets

from .ByteArray import ByteArray


class PrivateKey(ByteArray):
    """Represents a private key."""

    # pylint: disable=too-few-public-methods

    def __init__(self, private_key):
        """Creates a private key from bytes or a hex string."""
        super().__init__(32, private_key, PrivateKey)

    @staticmethod
    def random():
        """Generates a random private key."""
        return PrivateKey(secrets.token_bytes(32))


class PublicKey(ByteArray):
    """Represents a public key."""

    # pylint: disable=too-few-public-methods

    def __init__(self, public_key):
        """Creates a public key from bytes or a hex string."""
        super().__init__(32, public_key, PublicKey)


class Signature(ByteArray):
    """Represents a signature."""

    # pylint: disable=too-few-public-methods

    def __init__(self, signature):
        """Creates a signature from bytes or a hex string."""
        super().__init__(64, signature, Signature)
