import unittest
from binascii import hexlify

from symbolchain.CryptoTypes import PublicKey
from symbolchain.sc import AccountRestrictionFlags, LinkAction, NetworkType, TransactionType, UnresolvedAddress
from symbolchain.symbol.IdGenerator import generate_mosaic_id, generate_namespace_id
from symbolchain.symbol.Network import Address, Network
from symbolchain.symbol.TransactionFactory import TransactionFactory

from ..test.BasicTransactionFactoryTest import BasicTransactionFactoryTest
from ..test.NemTestUtils import NemTestUtils

FOO_NETWORK = Network('foo', 0x54)
TEST_SIGNER_PUBLIC_KEY = NemTestUtils.randcryptotype(PublicKey)


class SymbolTransactionFactoryTest(BasicTransactionFactoryTest):
	# pylint: disable=no-member
	# pylint: disable=abstract-method

	def _assert_transfer(self, transaction):
		self.assertEqual(TransactionType.TRANSFER, transaction.type_)
		self.assertEqual(1, transaction.version)
		self.assertEqual(NetworkType.TESTNET, transaction.network)

	def _assert_account_link(self, transaction):
		self.assertEqual(LinkAction.LINK, transaction.link_action)

	def create_factory(self, type_parsing_rules=None):
		return TransactionFactory(Network.TESTNET, type_parsing_rules)

	# region create

	def test_can_create_known_transaction_with_multiple_overrides(self):
		# Arrange:
		# there's subtle difference between the two:
		#  * hint on `hash` is pod:Hash256,
		#    type parsing rule that is getting created is for STRING 'Hash256'
		#  * same goes for `BlockDuration`
		#  * signer_public_key is handled differently in Transaction factory, via hint:
		#    {'signer_public_key': PublicKey} which maps it to SDK type CryptoTypes.PublicKey
		factory = self.create_factory({
			'Hash256': lambda x: x + ' a hash',
			'BlockDuration': lambda _: 654321,
			PublicKey: lambda address: address + ' PUBLICKEY'
		})

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'hash_lock_transaction',
			'signer_public_key': 'signer_name',
			'hash': 'not really',
			'duration': 'fake duration',
			'mosaic': {'mosaic_id': 0x12345678ABCDEF, 'amount': 12345}
		})

		# Assert:
		self.assertEqual(TransactionType.HASH_LOCK, transaction.type_)
		self.assertEqual(0x4148, transaction.type_.value)
		self.assertEqual(1, transaction.version)
		self.assertEqual(NetworkType.TESTNET, transaction.network)
		self.assertEqual('signer_name PUBLICKEY', transaction.signer_public_key)

		self.assertEqual(654321, transaction.duration)
		self.assertEqual('not really a hash', transaction.hash)
		self.assertEqual(0x12345678ABCDEF, transaction.mosaic.mosaic_id.value)
		self.assertEqual(12345, transaction.mosaic.amount.value)

	# endregion

	# region flags and enums handling

	def test_can_create_transaction_with_default_flags_handling(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'account_address_restriction_transaction',
			'signer_public_key': 'signer_name',
			'restriction_flags': 'address mosaic_id transaction_type outgoing block'
		})

		# Assert:
		Flags = AccountRestrictionFlags
		self.assertEqual(
			Flags.ADDRESS | Flags.MOSAIC_ID | Flags.TRANSACTION_TYPE | Flags.OUTGOING | Flags.BLOCK,
			transaction.restriction_flags)

	def test_can_create_transaction_with_default_enum_array_handling(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'account_operation_restriction_transaction',
			'signer_public_key': 'signer_name',
			'restriction_additions': [
				'transfer',
				'account_key_link'
			],
			'restriction_deletions': [
				'vrf_key_link',
				'voting_key_link'
			]
		})

		# Assert:
		expected_additions = [TransactionType.TRANSFER, TransactionType.ACCOUNT_KEY_LINK]
		self.assertEqual(expected_additions, transaction.restriction_additions)
		expected_deletions = [TransactionType.VRF_KEY_LINK, TransactionType.VOTING_KEY_LINK]
		self.assertEqual(expected_deletions, transaction.restriction_deletions)

	# endregion

	# region byte array type conversion

	def test_can_create_transaction_with_type_conversion(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'account_address_restriction_transaction',
			'signer_public_key': 'signer_name',
			'restriction_additions': [
				Address('AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA'),
				Address('DINRYHI6D4QCCIRDEQSSMJZIFEVCWLBNFYXTAMI')
			]
		})

		# Assert:
		self.assertEqual(
			[UnresolvedAddress(bytes(range(1, 25))), UnresolvedAddress(bytes(range(26, 50)))],
			transaction.restriction_additions)

	# endregion

	# region id autogeneration

	def test_can_autogenerate_namespace_registration_root_id(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'namespace_registration_transaction',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
			'registration_type': 'root',
			'duration': 123,
			'name': 'roger'
		})

		# Assert:
		expected_id = generate_namespace_id('roger')
		self.assertEqual(expected_id, transaction.id.value)

	def test_can_autogenerate_namespace_registration_child_id(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'namespace_registration_transaction',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
			'registration_type': 'child',
			'parent_id': generate_namespace_id('roger'),
			'name': 'charlie'
		})

		# Assert:
		expected_id = generate_namespace_id('charlie', generate_namespace_id('roger'))
		self.assertEqual(expected_id, transaction.id.value)

	def test_can_autogenerate_mosaic_definition_id(self):
		# Arrange:
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'mosaic_definition_transaction',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
			'duration': 1,
			'nonce': 123,
			'flags': 'transferable restrictable',
			'divisibility': 2
		})

		# Assert:
		expected_id = generate_mosaic_id(factory.network.public_key_to_address(PublicKey(TEST_SIGNER_PUBLIC_KEY)), 123)
		self.assertEqual(expected_id, transaction.id.value)

	# endregion


class EmbeddedTransactionFactoryTest(SymbolTransactionFactoryTest, unittest.TestCase):
	@property
	def supports_signature_test(self):
		return False

	def _assert_signature(self, transaction, signature, signed_transaction_buffer):
		raise NotImplementedError()

	def create_transaction(self, factory):
		return factory.create_embedded


class TransactionFactoryTest(SymbolTransactionFactoryTest, unittest.TestCase):
	def _assert_signature(self, transaction, signature, signed_transaction_buffer):
		serialized_transaction_hex = hexlify(transaction.serialize()).decode('utf8').upper()
		expected_buffer = f'{{"payload": "{serialized_transaction_hex}"}}'.encode('utf8')
		self.assertEqual(expected_buffer, signed_transaction_buffer)

	def create_transaction(self, factory):
		return factory.create
