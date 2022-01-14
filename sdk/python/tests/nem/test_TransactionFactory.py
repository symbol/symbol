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
from ..test.NemTestUtils import NemTestUtils

TEST_SIGNER_PUBLIC_KEY = NemTestUtils.randcryptotype(PublicKey)


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

	# region create

	def test_can_create_known_transaction_with_multiple_overrides(self):
		# Arrange:
		#  * hint on `rental_fee_recipient` is pod:Address,
		#   type parsing rule that is getting created is for STRING 'Address'
		# * same happens for `rental_fee`
		#  * signer_public_key is handled differently in Transaction factory, via hint:
		#    {'signer_public_key': PublicKey} which maps it to SDK type CryptoTypes.PublicKey
		factory = self.create_factory({
			'Address': lambda x: x + ' but amazing',
			'Amount': lambda _: 654321,
			PublicKey: lambda address: address + ' PUBLICKEY'
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
		self.assertEqual('signer_name PUBLICKEY', transaction.signer_public_key)

		self.assertEqual('fee sink but amazing', transaction.rental_fee_sink)
		self.assertEqual(654321, transaction.rental_fee)

	# endregion

	# region byte array type conversion

	def test_can_create_transaction_with_type_conversion(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'namespace_registration_transaction',
			'signer_public_key': 'signer_name',
			'rental_fee_sink': Address('AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGAB')
		})

		# Assert:
		self.assertEqual(
			nc_Address('4145424147424146415944515143494B424D474132445150434149524545595543554C424F474142'),
			transaction.rental_fee_sink)

	# endregion

	# region to_non_verifiable_transaction

	def test_to_non_verifiable_skips_signature(self):
		# Arrange:
		factory = self.create_factory()
		signature = NemTestUtils.randcryptotype(Signature)

		transaction = self.create_transaction(factory)({
			'type': 'transfer_transaction_v1',
			'timestamp': randint(0, (1 << (8 * Timestamp.SIZE)) - 1),
			'signer_public_key': NemTestUtils.randcryptotype(PublicKey),
			'signature': signature,
			'fee': randint(0, (1 << (8 * Amount.SIZE)) - 1),
			'deadline': randint(0, (1 << (8 * Timestamp.SIZE)) - 1),
			'recipient_address': NemTestUtils.randcryptotype(Address),
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
		offset = TransactionType.TRANSFER.size() + 1 + 2 + NetworkType.TESTNET.size() + Timestamp.SIZE + 4 + nc_PublicKey.SIZE
		expected_serialized = verifiable_serialized[:offset] + verifiable_serialized[offset + 4 + nc_Signature.SIZE:]
		self.assertEqual(expected_serialized, non_verifiable_transaction.serialize())

		# - additionally check that serialized signature matches initial one
		self.assertEqual(signature.bytes, verifiable_serialized[offset + 4: offset + 4 + nc_Signature.SIZE])

	# endregion
