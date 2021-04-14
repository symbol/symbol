import os

from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ed25519

from .CryptoTypes import PrivateKey


class PrivateKeyStorage:
    """Loads and saves private keys as PEM files with optional encryption in a directory."""

    def __init__(self, directory, password=None):
        """Creates storage for a directory."""
        self.directory = directory
        self.password = None if not password else password.encode('utf8')

    def save(self, name, private_key):
        """Saves a private key with the specified name."""
        wrapped_private_key = ed25519.Ed25519PrivateKey.from_private_bytes(private_key.bytes)
        with open(self._get_file_path(name), 'wb') as outfile:
            encryption_algorithm = serialization.NoEncryption()
            if self.password:
                encryption_algorithm = serialization.BestAvailableEncryption(self.password)

            outfile.write(wrapped_private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=encryption_algorithm))

    def load(self, name):
        """Loads a private key with the specified name."""
        with open(self._get_file_path(name), 'rb') as infile:
            wrapped_private_key = serialization.load_pem_private_key(infile.read(), password=self.password)
            private_key_bytes = wrapped_private_key.private_bytes(
                encoding=serialization.Encoding.Raw,
                format=serialization.PrivateFormat.Raw,
                encryption_algorithm=serialization.NoEncryption())
            return PrivateKey(private_key_bytes)

    def _get_file_path(self, name):
        return os.path.join(self.directory, '{}.pem'.format(name))
