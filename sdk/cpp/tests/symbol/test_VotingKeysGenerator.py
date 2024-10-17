import unittest

from symbolchain.BufferReader import BufferReader
from symbolchain.CryptoTypes import PrivateKey, PublicKey, Signature
from symbolchain.symbol.KeyPair import KeyPair, Verifier
from symbolchain.symbol.VotingKeysGenerator import VotingKeysGenerator


class VotingKeysGeneratorTest(unittest.TestCase):
	def test_can_generate_header(self):
		# Arrange:
		root_key_pair = KeyPair(PrivateKey.random())
		voting_keys_generator = VotingKeysGenerator(root_key_pair)

		# Act:
		voting_keys_buffer = voting_keys_generator.generate(7, 11)

		# Assert:
		self.assertEqual(32 + PublicKey.SIZE + 16 + 5 * (PrivateKey.SIZE + Signature.SIZE), len(voting_keys_buffer))

		reader = BufferReader(voting_keys_buffer)
		self.assertEqual(7, reader.read_int(8))
		self.assertEqual(11, reader.read_int(8))
		self.assertEqual(0xFFFFFFFFFFFFFFFF, reader.read_int(8))
		self.assertEqual(0xFFFFFFFFFFFFFFFF, reader.read_int(8))

		self.assertEqual(root_key_pair.public_key, PublicKey(reader.read_bytes(PublicKey.SIZE)))
		self.assertEqual(7, reader.read_int(8))
		self.assertEqual(11, reader.read_int(8))

	def test_can_generate_random_child_keys(self):
		# Arrange:
		root_key_pair = KeyPair(PrivateKey.random())
		voting_keys_generator = VotingKeysGenerator(root_key_pair)

		# Act:
		voting_keys_buffer = voting_keys_generator.generate(7, 11)

		# Assert:
		self.assertEqual(32 + PublicKey.SIZE + 16 + 5 * (PrivateKey.SIZE + Signature.SIZE), len(voting_keys_buffer))

		reader = BufferReader(voting_keys_buffer)
		reader.read_bytes(32 + PublicKey.SIZE + 16)  # skip header

		verifier = Verifier(root_key_pair.public_key)
		for i in reversed(range(0, 5)):
			child_private_key = PrivateKey(reader.read_bytes(PrivateKey.SIZE))
			signature = Signature(reader.read_bytes(Signature.SIZE))

			child_key_pair = KeyPair(child_private_key)
			signed_payload = child_key_pair.public_key.bytes + (7 + i).to_bytes(8, 'little')

			self.assertTrue(verifier.verify(signed_payload, signature), f'child at {i}')
