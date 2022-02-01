import unittest
from binascii import hexlify
from random import randint

from symbolchain import nc
from symbolchain.CryptoTypes import PublicKey, Signature
from symbolchain.nem.Network import Address, Network
from symbolchain.nem.TransactionFactory import TransactionFactory

from ..test.BasicTransactionFactoryTest import BasicTransactionFactoryTest
from ..test.TestUtils import TestUtils

TEST_SIGNER_PUBLIC_KEY = TestUtils.random_byte_array(PublicKey)


class TransactionFactoryTest(BasicTransactionFactoryTest, unittest.TestCase):
	def assert_transaction(self, transaction):
		self.assertEqual(nc.TransactionType.TRANSFER, transaction.type_)
		self.assertEqual(2, transaction.version)
		self.assertEqual(nc.NetworkType.TESTNET, transaction.network)

	def create_factory(self, type_rule_overrides=None):
		return TransactionFactory(Network.TESTNET, type_rule_overrides)

	def assert_signature(self, transaction, signature, signed_transaction_payload):
		transaction_hex = hexlify(TransactionFactory.to_non_verifiable_transaction(transaction).serialize()).decode('utf8').upper()
		signature_hex = str(signature)
		expected_json_string = f'{{"data":"{transaction_hex}", "signature":"{signature_hex}"}}'
		self.assertEqual(expected_json_string, signed_transaction_payload)

	# region rules

	def test_rules_contain_expected_hints(self):
		# Act:
		factory = factory = self.create_factory()

		# Assert:
		expected_rule_names = [
			'Amount', 'Height', 'Timestamp',

			'BlockType', 'LinkAction', 'MessageType', 'MosaicSupplyChangeAction', 'MosaicTransferFeeType',
			'MultisigAccountModificationType', 'NetworkType', 'TransactionType',

			'struct:Cosignature',
			'struct:Message',
			'struct:Mosaic',
			'struct:MosaicDefinition',
			'struct:MosaicId',
			'struct:MosaicLevy',
			'struct:MosaicProperty',
			'struct:MultisigAccountModification',
			'struct:NamespaceId',
			'struct:SizePrefixedCosignature',
			'struct:SizePrefixedMosaic',
			'struct:SizePrefixedMosaicProperty',
			'struct:SizePrefixedMultisigAccountModification',

			'Address', 'Hash256', 'PublicKey',

			'array[SizePrefixedCosignature]',
			'array[SizePrefixedMosaic]',
			'array[SizePrefixedMosaicProperty]',
			'array[SizePrefixedMultisigAccountModification]'
		]
		self.assertEqual(set(expected_rule_names), set(factory.factory.rules.keys()))

	# endregion

	# region create

	def test_can_create_known_transaction_with_multiple_overrides(self):
		# Arrange:
		factory = self.create_factory({
			Address: lambda x: f'{x} but amazing',
			nc.Amount: lambda _: 654321,
			PublicKey: lambda x: f'{x} PUBLICKEY'
		})

		# Act:
		transaction = factory.create({
			'type': 'namespace_registration_transaction',
			'signer_public_key': 'signer_name',
			'rental_fee_sink': 'fee sink',
			'rental_fee': 'fake fee'
		})

		# Assert:
		self.assertEqual(nc.TransactionType.NAMESPACE_REGISTRATION, transaction.type_)
		self.assertEqual(1, transaction.version)
		self.assertEqual(nc.NetworkType.TESTNET, transaction.network)

		self.assertEqual(b'signer_name PUBLICKEY', transaction.signer_public_key)
		self.assertEqual(b'fee sink but amazing', transaction.rental_fee_sink)
		self.assertEqual(654321, transaction.rental_fee)

	# endregion

	# region address type conversion

	def test_can_create_transaction_with_address(self):
		# Arrange: this tests the custom type converter
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'namespace_registration_transaction',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
			'rental_fee_sink': Address('AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGAB')
		})

		# Assert:
		self.assertEqual(
			nc.Address('4145424147424146415944515143494B424D474132445150434149524545595543554C424F474142'),
			transaction.rental_fee_sink)

	# endregion

	# region message encoding

	def test_can_create_transfer_with_string_message(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'transfer_transaction',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
			'message': {
				'message_type': 'plain',
				'message': 'You miss 100%% of the shots you don\'t take'
			}
		})

		# Assert:
		self.assertEqual(b'You miss 100%% of the shots you don\'t take', transaction.message.message)

	# endregion

	# region non verifiable

	@staticmethod
	def _create_transfer_descriptor_with_signature(signature):
		def generate_random_value(model_type):
			return randint(0, (1 << (8 * model_type.SIZE)) - 1)

		return {
			'type': 'transfer_transaction_v1',
			'timestamp': generate_random_value(nc.Timestamp),
			'signer_public_key': TestUtils.random_byte_array(PublicKey),
			'signature': signature,
			'fee': generate_random_value(nc.Amount),
			'deadline': generate_random_value(nc.Timestamp),
			'recipient_address': TestUtils.random_byte_array(Address),
			'amount': generate_random_value(nc.Amount),
			'message': {
				'message_type': 'plain',
				'message': 'Wayne Gretzky'
			}
		}

	def test_can_convert_verifiable_transaction_to_non_verifiable(self):
		# Arrange:
		factory = self.create_factory()
		signature = TestUtils.random_byte_array(Signature)
		transaction = self.create_transaction(factory)(self._create_transfer_descriptor_with_signature(signature))

		# Act:
		non_verifiable_transaction = TransactionFactory.to_non_verifiable_transaction(transaction)

		# Assert:
		self.assertFalse(hasattr(non_verifiable_transaction, 'signature'))

		# - cut out size and signature from the buffer
		verifiable_buffer = transaction.serialize()
		offset = nc.TransactionType.TRANSFER.size + 1 + 2 + nc.NetworkType.TESTNET.size + nc.Timestamp.SIZE + 4 + nc.PublicKey.SIZE
		expected_non_verifiable_buffer = verifiable_buffer[:offset] + verifiable_buffer[offset + 4 + nc.Signature.SIZE:]
		self.assertEqual(expected_non_verifiable_buffer, non_verifiable_transaction.serialize())

		# - additionally check that serialized signature matches initial one
		self.assertEqual(signature.bytes, verifiable_buffer[offset + 4: offset + 4 + nc.Signature.SIZE])

	def test_can_convert_non_verifiable_transaction_to_non_verifiable(self):
		# Arrange:
		factory = self.create_factory()
		signature = TestUtils.random_byte_array(Signature)
		transaction = self.create_transaction(factory)(self._create_transfer_descriptor_with_signature(signature))

		non_verifiable_transaction1 = TransactionFactory.to_non_verifiable_transaction(transaction)

		# Act:
		non_verifiable_transaction2 = TransactionFactory.to_non_verifiable_transaction(non_verifiable_transaction1)

		# Assert:
		self.assertEqual(non_verifiable_transaction1.serialize(), non_verifiable_transaction2.serialize())

	# endregion
