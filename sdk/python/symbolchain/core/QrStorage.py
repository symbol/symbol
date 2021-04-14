import base64
import os

import qrcode
from PIL import Image
from pyzbar import pyzbar


class QrStorage:
    """Loads and saves binary data as QR codes in a directory."""

    def __init__(self, directory):
        """Creates storage for a directory."""
        self.directory = directory

    def save(self, name, buffer):
        """Saves a buffer."""
        qrcode_payload = self._binary_to_qrcode_payload(buffer)
        qrcode_builder = qrcode.QRCode(error_correction=qrcode.constants.ERROR_CORRECT_H)
        qrcode_builder.add_data(qrcode.util.QRData(qrcode_payload, qrcode.util.MODE_ALPHA_NUM))
        qrcode_builder.make(fit=True)

        qrcode_image = qrcode_builder.make_image(fill_color='#44004E', back_color='#FFFFFF')
        qrcode_image.save(self._get_file_path(name))

    def load(self, name):
        """Loads a buffer."""
        # [0] because exactly one qrcode is expected in the image
        qrcode_payload = pyzbar.decode(Image.open(self._get_file_path(name)), symbols=[pyzbar.ZBarSymbol.QRCODE])[0].data
        return self._qrcode_payload_to_binary(qrcode_payload)

    @staticmethod
    def _binary_to_qrcode_payload(buffer):
        # replace '=' with a character in QR code alphanumeric alphabet, which enables more efficient data packing in the qrcode
        return base64.b32encode(buffer).decode('utf8').replace('=', '$').encode('utf8')

    @staticmethod
    def _qrcode_payload_to_binary(qrcode_payload):
        return base64.b32decode(qrcode_payload.decode('utf8').replace('$', '='))

    def _get_file_path(self, name):
        return os.path.join(self.directory, '{}.png'.format(name))
