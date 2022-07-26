from ..BufferWriter import BufferWriter
from ..CryptoTypes import PrivateKey
from .KeyPair import KeyPair


class VotingKeysGenerator:
	"""Generates symbol voting keys."""

	def __init__(self, root_key_pair, private_key_generator=PrivateKey.random):
		"""Creates a generator around a voting root key pair."""
		self.root_key_pair = root_key_pair
		self.private_key_generator = private_key_generator

	def generate(self, start_epoch, end_epoch):
		"""Generates voting keys for specified epochs."""
		writer = BufferWriter()
		writer.write_int(start_epoch, 8)  # start key identifier
		writer.write_int(end_epoch, 8)  # end key identifier
		writer.write_int(0xFFFFFFFFFFFFFFFF, 8)  # reserved - last (used) key identifier
		writer.write_int(0xFFFFFFFFFFFFFFFF, 8)  # reserved - last wiped key identifier

		writer.write_bytes(self.root_key_pair.public_key.bytes)  # root voting public key
		writer.write_int(start_epoch, 8)  # level 1/1 start key identifier
		writer.write_int(end_epoch, 8)  # level 1/1 end key identifier

		for identifier in reversed(range(start_epoch, end_epoch + 1)):
			child_private_key = self.private_key_generator()
			child_key_pair = KeyPair(child_private_key)

			parent_signed_payload_writer = BufferWriter()
			parent_signed_payload_writer.write_bytes(child_key_pair.public_key.bytes)
			parent_signed_payload_writer.write_int(identifier, 8)
			signature = self.root_key_pair.sign(parent_signed_payload_writer.buffer)

			writer.write_bytes(child_key_pair.private_key.bytes)  # child voting private key used to sign votes for an epoch
			writer.write_bytes(signature.bytes)  # signature proving derivation of child key pair from root

		return writer.buffer
