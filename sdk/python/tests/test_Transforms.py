import unittest
from binascii import unhexlify

from symbolchain.Transforms import ripemd_keccak_256


class TransformsTest(unittest.TestCase):
	def test_can_transform_with_ripemd_keccak_256(self):
		# Arrange:
		payload = unhexlify('BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F')

		# Act:
		hash_result = ripemd_keccak_256(payload)

		# Assert:
		self.assertEqual(unhexlify('FDB8D529F3656230A7FD6F183A0E8D750E4033C3'), hash_result)
