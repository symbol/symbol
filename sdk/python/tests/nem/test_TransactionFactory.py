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

	@staticmethod
	def transaction_type_name():
		return 'transfer_transaction_v2'

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

			'LinkAction', 'MessageType', 'MosaicSupplyChangeAction', 'MosaicTransferFeeType',
			'MultisigAccountModificationType', 'NetworkType', 'TransactionType',

			'struct:CosignatureV1',
			'struct:Message',
			'struct:Mosaic',
			'struct:MosaicDefinition',
			'struct:MosaicId',
			'struct:MosaicLevy',
			'struct:MosaicProperty',
			'struct:MultisigAccountModification',
			'struct:NamespaceId',
			'struct:SizePrefixedCosignatureV1',
			'struct:SizePrefixedMosaic',
			'struct:SizePrefixedMosaicProperty',
			'struct:SizePrefixedMultisigAccountModification',

			'Address', 'Hash256', 'PublicKey',

			'array[SizePrefixedCosignatureV1]',
			'array[SizePrefixedMosaic]',
			'array[SizePrefixedMosaicProperty]',
			'array[SizePrefixedMultisigAccountModification]'
		]
		self.assertEqual(set(expected_rule_names), set(factory.factory.rules.keys()))

	# endregion

	# region lookup_transaction_name

	def test_lookup_transaction_name_can_lookup_known_transaction(self):
		self.assertEqual('transfer_transaction_v1', TransactionFactory.lookup_transaction_name(nc.TransactionType.TRANSFER, 1))
		self.assertEqual('transfer_transaction_v2', TransactionFactory.lookup_transaction_name(nc.TransactionType.TRANSFER, 2))
		self.assertEqual('multisig_transaction_v1', TransactionFactory.lookup_transaction_name(nc.TransactionType.MULTISIG, 1))

	def test_lookup_transaction_name_cannot_lookup_unknown_transaction(self):
		with self.assertRaises(ValueError):
			TransactionFactory.lookup_transaction_name(nc.TransactionType(123), 1)

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
			'type': 'namespace_registration_transaction_v1',
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
			'type': 'namespace_registration_transaction_v1',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
			'rental_fee_sink': Address('AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGAB')
		})

		# Assert:
		self.assertEqual(
			nc.Address('4145424147424146415944515143494B424D474132445150434149524545595543554C424F474142'),
			transaction.rental_fee_sink)

	# endregion

	# region sorting

	@staticmethod
	def _create_unordered_descriptor():
		return {
			'type': 'multisig_account_modification_transaction_v2',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
			'modifications': [
				{
					'modification': {
						'modification_type': 'delete_cosignatory',
						'cosignatory_public_key': PublicKey('D79936328C188A4416224ABABF580CA2C5C8D852248DB1933FE4BC0DCA0EE7BC')
					}
				},
				{
					'modification': {
						'modification_type': 'add_cosignatory',
						'cosignatory_public_key': PublicKey('5D378657691CAD70CE35A46FB88CB134232B0B6B3655449C019A1F5F20AE9AAD')
					}
				}
			]
		}

	def test_can_create_transaction_with_out_of_order_array_when_autosort_is_enabled(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)(self._create_unordered_descriptor())

		# Assert: modifications were reordered
		self.assertEqual(nc.MultisigAccountModificationType.ADD_COSIGNATORY, transaction.modifications[0].modification.modification_type)
		self.assertEqual(nc.MultisigAccountModificationType.DELETE_COSIGNATORY, transaction.modifications[1].modification.modification_type)

	def test_cannot_create_transaction_with_out_of_order_array_when_autosort_is_disabled(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)(self._create_unordered_descriptor(), autosort=False)

		# Assert: modifications were NOT reordered (serialization will fail)
		self.assertEqual(nc.MultisigAccountModificationType.DELETE_COSIGNATORY, transaction.modifications[0].modification.modification_type)
		self.assertEqual(nc.MultisigAccountModificationType.ADD_COSIGNATORY, transaction.modifications[1].modification.modification_type)

		with self.assertRaises(ValueError):
			transaction.serialize()

	# endregion

	# region message encoding

	def test_can_create_transfer_with_string_message(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'transfer_transaction_v2',
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
