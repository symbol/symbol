from abc import abstractmethod

from symbolchain.CryptoTypes import PublicKey, Signature

from .TestUtils import TestUtils

TEST_SIGNER_PUBLIC_KEY = TestUtils.random_byte_array(PublicKey)


class AbstractBasicTransactionFactoryExSignatureTest:
	@abstractmethod
	def assert_transaction(self, transaction):
		pass

	@abstractmethod
	def create_factory(self, type_rule_overrides=None):
		pass

	@staticmethod
	def transaction_type_name():
		return 'transfer_transaction_v1'

	@staticmethod
	def create_transaction(factory):
		return factory.create


class BasicTransactionFactoryExSignatureTest(AbstractBasicTransactionFactoryExSignatureTest):
	# pylint: disable=abstract-method, no-member

	# region create

	def test_can_create_known_transaction_from_descriptor(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': self.transaction_type_name(),
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY
		})

		# Assert:
		self.assert_transaction(transaction)
		self.assertEqual(TEST_SIGNER_PUBLIC_KEY.bytes, transaction.signer_public_key.bytes)

	def test_cannot_create_unknown_transaction_from_descriptor(self):
		# Arrange:
		factory = self.create_factory()

		# Act + Assert:
		with self.assertRaises(ValueError):
			self.create_transaction(factory)({
				'type': f'x{self.transaction_type_name()}',
				'signer_public_key': TEST_SIGNER_PUBLIC_KEY
			})

	# endregion


class BasicTransactionFactoryTest(BasicTransactionFactoryExSignatureTest):
	# pylint: disable=no-member

	# region attach_signature

	def test_can_attach_signature_to_transaction(self):
		# Arrange:
		factory = self.create_factory()
		transaction = self.create_transaction(factory)({
			'type': self.transaction_type_name(),
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY
		})
		signature = TestUtils.random_byte_array(Signature)

		# Sanity:
		self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

		# Act:
		signed_transaction_payload = factory.attach_signature(transaction, signature)

		# Assert:
		self.assert_transaction(transaction)
		self.assertEqual(TEST_SIGNER_PUBLIC_KEY.bytes, transaction.signer_public_key.bytes)
		self.assertEqual(signature.bytes, transaction.signature.bytes)

		self.assert_signature(transaction, signature, signed_transaction_payload)

	# endregion

	@abstractmethod
	def assert_signature(self, transaction, signature, signed_transaction_payload):
		pass
