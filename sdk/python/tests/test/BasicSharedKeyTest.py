from collections import namedtuple

from symbolchain.CryptoTypes import PrivateKey, PublicKey

SharedKeyTestDescriptor = namedtuple('SharedKeyTestDescriptor', ['key_pair_class', 'derive_shared_key'])


class BasicSharedKeyTest:
	# pylint: disable=no-member

	# region shared key

	def _assert_derived_shared_result(self, mutate, comparator):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		private_key_1 = PrivateKey.random()
		other_public_key_1 = test_descriptor.key_pair_class(PrivateKey.random()).public_key

		private_key_2_bytes = bytearray(private_key_1.bytes)
		other_public_key_2_bytes = bytearray(other_public_key_1.bytes)

		mutate(private_key_2_bytes, other_public_key_2_bytes)

		key_pair_1 = test_descriptor.key_pair_class(private_key_1)
		key_pair_2 = test_descriptor.key_pair_class(PrivateKey(bytes(private_key_2_bytes)))

		# Act:
		shared_key_1 = test_descriptor.derive_shared_key(key_pair_1, other_public_key_1)
		shared_key_2 = test_descriptor.derive_shared_key(key_pair_2, PublicKey(bytes(other_public_key_2_bytes)))

		# Assert:
		comparator(shared_key_1, shared_key_2)

	def test_shared_keys_generated_with_same_inputs_are_equal(self):
		self._assert_derived_shared_result(lambda _x, _y: None, self.assertEqual)

	def test_shared_keys_generated_for_different_private_keys_are_different(self):
		def mutate_private_key(private_key_bytes, _):
			private_key_bytes[0] = private_key_bytes[0] ^ 0xFF

		self._assert_derived_shared_result(mutate_private_key, self.assertNotEqual)

	def test_shared_keys_generated_for_different_other_public_keys_are_different(self):
		def create_mutator(i):
			def mutate_other_public_key(_, other_public_key):
				other_public_key[0] = other_public_key[0] ^ i

			return mutate_other_public_key

		# this test needs to be fired multiple times, so that mutated public key will not to be rejected via subgroup check
		for i in range(1, 256):
			try:
				self._assert_derived_shared_result(create_mutator(i), self.assertNotEqual)
				break
			except ValueError:
				pass

	# endregion

	# region mutual shared

	def test_mutual_shared_results_are_equal(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()
		key_pair_1 = test_descriptor.key_pair_class(PrivateKey.random())
		key_pair_2 = test_descriptor.key_pair_class(PrivateKey.random())

		# Act:
		shared_key_1 = test_descriptor.derive_shared_key(key_pair_1, key_pair_2.public_key)
		shared_key_2 = test_descriptor.derive_shared_key(key_pair_2, key_pair_1.public_key)

		# Assert:
		self.assertEqual(shared_key_1, shared_key_2)

	# endregion

	# region invalid public key

	def test_public_key_not_on_the_curve_throws(self):
		# Arrange:
		test_descriptor = self.get_test_descriptor()

		key_pair_1 = test_descriptor.key_pair_class(PrivateKey.random())
		key_pair_2 = test_descriptor.key_pair_class(PrivateKey.random())

		# Act + Assert:
		# this test needs to be fired multiple times, because just setting byte to 1
		# might end up in a point that will still land on the curve
		# note: cannot use `self.assertRaises` below, as it behaves in a weird way inside try/except block
		for i in range(4):
			invalid_public_key_bytes = bytearray(key_pair_2.public_key.bytes)
			invalid_public_key_bytes[31] = i

			succeeded = False
			try:
				test_descriptor.derive_shared_key(key_pair_1, PublicKey(bytes(invalid_public_key_bytes)))
			except ValueError as err:
				if 'point is not in main subgroup' in str(err) or 'decoding point that is not on curve' in str(err):
					succeeded = True

			if succeeded:
				break

		self.assertTrue(succeeded)

	# endregion
