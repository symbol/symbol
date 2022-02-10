import unittest
from binascii import hexlify
from random import randint

from symbolchain.CryptoTypes import PublicKey, Signature
from symbolchain.nc import Address as nc_Address
from symbolchain.nc import Amount, LinkAction, NetworkType
from symbolchain.nc import PublicKey as nc_PublicKey
from symbolchain.nc import Signature as nc_Signature
from symbolchain.nc import Timestamp, TransactionType
from symbolchain.nem.Network import Address, Network
from symbolchain.nem.TransactionFactory import TransactionFactory

from ..test.BasicTransactionFactoryTest import BasicTransactionFactoryTest
from ..test.TestUtils import TestUtils

TEST_SIGNER_PUBLIC_KEY = TestUtils.random_byte_array(PublicKey)


class TransactionFactoryTest(BasicTransactionFactoryTest, unittest.TestCase):
	def _assert_transfer(self, transaction):
		self.assertEqual(TransactionType.TRANSFER, transaction.type_)
		self.assertEqual(2, transaction.version)
		self.assertEqual(NetworkType.TESTNET, transaction.network)

	def _assert_account_link(self, transaction):
		self.assertEqual(LinkAction.LINK, transaction.link_action)

	def _assert_signature(self, transaction, signature, signed_transaction_buffer):
		transaction_hex = hexlify(TransactionFactory.to_non_verifiable_transaction(transaction).serialize()).decode('utf8').upper()
		signature_hex = hexlify(signature.bytes).decode('utf8').upper()
		expected_buffer = f'{{"data":"{transaction_hex}", "signature":"{signature_hex}"}}'.encode('utf8')
		self.assertEqual(expected_buffer, signed_transaction_buffer)

	def create_factory(self, type_parsing_rules=None):
		return TransactionFactory(Network.TESTNET, type_parsing_rules)

	def create_transaction(self, factory):
		return factory.create

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
			Amount: lambda _: 654321,
			PublicKey: lambda address: f'{address} PUBLICKEY'
		})

		# Act:
		transaction = factory.create({
			'type': 'namespace_registration_transaction',
			'signer_public_key': 'signer_name',
			'rental_fee_sink': 'fee sink',
			'rental_fee': 'fake fee'
		})

		# Assert:
		self.assertEqual(TransactionType.NAMESPACE_REGISTRATION, transaction.type_)
		self.assertEqual(1, transaction.version)
		self.assertEqual(NetworkType.TESTNET, transaction.network)
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
			nc_Address('4145424147424146415944515143494B424D474132445150434149524545595543554C424F474142'),
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

	# region to_non_verifiable_transaction

	def test_to_non_verifiable_skips_signature(self):
		# Arrange:
		factory = self.create_factory()
		signature = TestUtils.random_byte_array(Signature)

		transaction = self.create_transaction(factory)({
			'type': 'transfer_transaction_v1',
			'timestamp': randint(0, (1 << (8 * Timestamp.SIZE)) - 1),
			'signer_public_key': TestUtils.random_byte_array(PublicKey),
			'signature': signature,
			'fee': randint(0, (1 << (8 * Amount.SIZE)) - 1),
			'deadline': randint(0, (1 << (8 * Timestamp.SIZE)) - 1),
			'recipient_address': TestUtils.random_byte_array(Address),
			'amount': randint(0, (1 << (8 * Amount.SIZE)) - 1),
			'message': {
				'message_type': 'plain',
				'message': ' Wayne Gretzky'.encode('utf8')
			}
		},)

		# Act:
		non_verifiable_transaction = TransactionFactory.to_non_verifiable_transaction(transaction)

		# Assert:
		self.assertFalse(hasattr(non_verifiable_transaction, 'signature'))

		# - cut out size and signature from the buffer
		verifiable_serialized = transaction.serialize()
		offset = TransactionType.TRANSFER.size + 1 + 2 + NetworkType.TESTNET.size + Timestamp.SIZE + 4 + nc_PublicKey.SIZE
		expected_serialized = verifiable_serialized[:offset] + verifiable_serialized[offset + 4 + nc_Signature.SIZE:]
		self.assertEqual(expected_serialized, non_verifiable_transaction.serialize())

		# - additionally check that serialized signature matches initial one
		self.assertEqual(signature.bytes, verifiable_serialized[offset + 4: offset + 4 + nc_Signature.SIZE])

	# endregion
