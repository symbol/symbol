import base64
import os

import qrcode
from PIL import Image
from pyzbar import pyzbar

from .BufferWriter import BufferWriter
from .CryptoTypes import Hash256, Signature


class QrSignatureStorage:
    """Loads and saves signatures as QR codes in a directory."""

    def __init__(self, directory):
        """Creates storage for a directory."""
        self.directory = directory

    def save(self, name, transaction_hash, signatures):
        """Saves a transaction hash along with attesting signatures."""
        writer = BufferWriter()
        writer.write_bytes(transaction_hash.bytes)
        for signature in signatures:
            writer.write_bytes(signature.bytes)

        qrcode_payload = self._binary_to_qrcode_payload(writer.buffer)
        qrcode_builder = qrcode.QRCode(error_correction=qrcode.constants.ERROR_CORRECT_H)
        qrcode_builder.add_data(qrcode.util.QRData(qrcode_payload, qrcode.util.MODE_ALPHA_NUM))
        qrcode_builder.make(fit=True)

        qrcode_image = qrcode_builder.make_image(fill_color='#44004E', back_color='#FFFFFF')
        qrcode_image.save(self._get_file_path(name))

    def load(self, name):
        """Loads a transaction hash along with attesting signatures."""
        # [0] because exactly one qrcode is expected in the image
        qrcode_payload = pyzbar.decode(Image.open(self._get_file_path(name)), symbols=[pyzbar.ZBarSymbol.QRCODE])[0].data
        decoded_buffer = self._qrcode_payload_to_binary(qrcode_payload)

        decoded_buffer_size = len(decoded_buffer)
        if decoded_buffer_size < Hash256.SIZE or 0 != (decoded_buffer_size - Hash256.SIZE) % Signature.SIZE:
            raise ValueError('decoded buffer from QR code has unexpected size {}'.format(decoded_buffer_size))

        transaction_hash = Hash256(decoded_buffer[0:Hash256.SIZE])

        signatures = []
        for i in range(0, (decoded_buffer_size - Hash256.SIZE) // Signature.SIZE):
            signature_start = Hash256.SIZE + i * Signature.SIZE
            signatures.append(Signature(decoded_buffer[signature_start:signature_start + Signature.SIZE]))

        return (transaction_hash, signatures)

    @staticmethod
    def _binary_to_qrcode_payload(buffer):
        # replace '=' with a character in QR code alphanumeric alphabet, which enables more efficient data packing in the qrcode
        return base64.b32encode(buffer).decode('utf8').replace('=', '$').encode('utf8')

    @staticmethod
    def _qrcode_payload_to_binary(qrcode_payload):
        return base64.b32decode(qrcode_payload.decode('utf8').replace('$', '='))

    def _get_file_path(self, name):
        return os.path.join(self.directory, '{}.png'.format(name))
