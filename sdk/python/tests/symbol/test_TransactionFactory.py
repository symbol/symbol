import unittest
from binascii import hexlify

from symbolchain.CryptoTypes import Hash256, PublicKey
from symbolchain.sc import BlockDuration, LinkAction, NetworkType, TransactionType, UnresolvedAddress
from symbolchain.symbol.IdGenerator import generate_mosaic_id, generate_namespace_id
from symbolchain.symbol.Network import Address, Network
from symbolchain.symbol.TransactionFactory import TransactionFactory

from ..test.BasicTransactionFactoryTest import BasicTransactionFactoryTest
from ..test.TestUtils import TestUtils

FOO_NETWORK = Network('foo', 0x54)
TEST_SIGNER_PUBLIC_KEY = TestUtils.random_byte_array(PublicKey)


class SymbolTransactionFactoryTest(BasicTransactionFactoryTest):
	# pylint: disable=abstract-method, no-member

	def _assert_transfer(self, transaction):
		self.assertEqual(TransactionType.TRANSFER, transaction.type_)
		self.assertEqual(1, transaction.version)
		self.assertEqual(NetworkType.TESTNET, transaction.network)

	def _assert_account_link(self, transaction):
		self.assertEqual(LinkAction.LINK, transaction.link_action)

	def create_factory(self, type_parsing_rules=None):
		return TransactionFactory(Network.TESTNET, type_parsing_rules)

	# region rules

	def test_rules_contain_expected_hints(self):
		# Act:
		factory = factory = self.create_factory()

		# Assert:
		expected_rule_names = [
			'Amount', 'BlockDuration', 'BlockFeeMultiplier', 'Difficulty', 'FinalizationEpoch', 'FinalizationPoint', 'Height', 'Importance',
			'ImportanceHeight', 'MosaicId', 'MosaicNonce', 'MosaicRestrictionKey', 'NamespaceId', 'ScopedMetadataKey', 'Timestamp',
			'UnresolvedMosaicId',

			'MosaicFlags', 'AccountRestrictionFlags', 'TransactionType', 'AliasAction', 'LinkAction', 'LockHashAlgorithm',
			'MosaicRestrictionType', 'MosaicSupplyChangeAction', 'NamespaceRegistrationType',

			'struct:UnresolvedMosaic',

			'UnresolvedAddress', 'Address', 'Hash256', 'PublicKey', 'VotingPublicKey',

			'array[UnresolvedMosaicId]', 'array[TransactionType]', 'array[UnresolvedAddress]', 'array[UnresolvedMosaic]',

			'NamespaceAliasType', 'MosaicRestrictionEntryType', 'NetworkType', 'LockStatus', 'MetadataType', 'BlockType',
			'AccountType', 'AccountStateFormat', 'ReceiptType', 'AccountKeyTypeFlags'
		]
		self.assertEqual(set(expected_rule_names), set(factory.factory.rules.keys()))

	# endregion

	# region create

	def test_can_create_known_transaction_with_multiple_overrides(self):
		# Arrange:
		factory = self.create_factory({
			Hash256: lambda x: f'{x} a hash',
			BlockDuration: lambda _: 654321,
			PublicKey: lambda x: f'{x} PUBLICKEY'
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
		self.assertEqual(b'signer_name PUBLICKEY', transaction.signer_public_key)

		self.assertEqual(654321, transaction.duration)
		self.assertEqual(b'not really a hash', transaction.hash)
		self.assertEqual(0x12345678ABCDEF, transaction.mosaic.mosaic_id.value)
		self.assertEqual(12345, transaction.mosaic.amount.value)

	# endregion

	# region address type conversion

	def test_can_create_transaction_with_address(self):
		# Arrange: this tests the custom type converter
		factory = self.create_factory()

		# Act:
		transaction = self.create_transaction(factory)({
			'type': 'account_address_restriction_transaction',
			'signer_public_key': TEST_SIGNER_PUBLIC_KEY,
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
