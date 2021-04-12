from .BufferWriter import BufferWriter
from .CryptoTypes import Hash256, Signature
from .QrStorage import QrStorage


class QrSignatureStorage:
    """Loads and saves signatures as QR codes in a directory."""

    def __init__(self, directory):
        """Creates storage for a directory."""
        self.storage = QrStorage(directory)

    def save(self, name, transaction_hash, signatures):
        """Saves a transaction hash along with attesting signatures."""
        writer = BufferWriter()
        writer.write_bytes(transaction_hash.bytes)
        for signature in signatures:
            writer.write_bytes(signature.bytes)

        self.storage.save(name, writer.buffer)

    def load(self, name):
        """Loads a transaction hash along with attesting signatures."""
        decoded_buffer = self.storage.load(name)

        decoded_buffer_size = len(decoded_buffer)
        if decoded_buffer_size < Hash256.SIZE or 0 != (decoded_buffer_size - Hash256.SIZE) % Signature.SIZE:
            raise ValueError('decoded buffer from QR code has unexpected size {}'.format(decoded_buffer_size))

        transaction_hash = Hash256(decoded_buffer[0:Hash256.SIZE])

        signatures = []
        for i in range(0, (decoded_buffer_size - Hash256.SIZE) // Signature.SIZE):
            signature_start = Hash256.SIZE + i * Signature.SIZE
            signatures.append(Signature(decoded_buffer[signature_start:signature_start + Signature.SIZE]))

        return (transaction_hash, signatures)
