from abc import abstractmethod

from symbolchain.CryptoTypes import PublicKey, Signature

from .NemTestUtils import NemTestUtils

TEST_SIGNER_PUBLIC_KEY = NemTestUtils.randcryptotype(PublicKey)


class BasicTransactionFactoryTest:
	# pylint: disable=no-member

	# region create

	def test_can_create_known_transaction_from_descriptor(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'transfer_transaction',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY
		})

		# Assert:
		self._assert_transfer(transaction)
		self.assertEqual(TEST_SIGNER_PUBLIC_KEY.bytes, transaction.signer_public_key.bytes)

	def test_cannot_create_unknown_transaction_from_descriptor(self):
		# Arrange:
		factory = self.create_factory()

		# Act + Assert:
		with self.assertRaises(ValueError):
			self.create_transaction(factory)({
				'type': 'xtransfer_transaction',
				'signer_public_key': TEST_SIGNER_PUBLIC_KEY
			})

	# endregion

	# region enums handling

	def test_can_create_transaction_with_default_enum_handling(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'account_key_link_transaction',
			'signer_public_key': 'signer_name',
			'link_action': 'link',
		})

		# Assert:
		self._assert_account_link(transaction)

	# endregion

	# region attach_signature

	def test_can_attach_signature_to_transaction(self):
		# Arrange:
		factory = self.create_factory()
		transaction = self.create_transaction(factory)({
			'type': 'transfer_transaction',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY
		})
		signature = NemTestUtils.randcryptotype(Signature)

		if not self.supports_signature_test:
			return

		# Sanity:
		self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

		# Act:
		signed_transaction_buffer = factory.attach_signature(transaction, signature)

		# Assert:
		self._assert_transfer(transaction)
		self.assertEqual(TEST_SIGNER_PUBLIC_KEY.bytes, transaction.signer_public_key.bytes)

		self.assertEqual(signature.bytes, transaction.signature.bytes)
		self._assert_signature(transaction, signature, signed_transaction_buffer)

	# endregion

	@property
	def supports_signature_test(self):
		return True

	@abstractmethod
	def _assert_transfer(self, transaction):
		pass

	@abstractmethod
	def _assert_account_link(self, transaction):
		pass

	@abstractmethod
	def _assert_signature(self, transaction, signature, signed_transaction_buffer):
		pass

	@abstractmethod
	def create_factory(self, type_parsing_rules=None):
		pass

	@abstractmethod
	def create_transaction(self, factory):
		pass
