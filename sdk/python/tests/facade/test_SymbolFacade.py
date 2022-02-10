import unittest

from symbolchain.AccountDescriptorRepository import AccountDescriptorRepository
from symbolchain.Bip32 import Bip32
from symbolchain.CryptoTypes import Hash256, PrivateKey, PublicKey, Signature
from symbolchain.facade.SymbolFacade import SymbolFacade

from ..test.TestUtils import TestUtils

YAML_INPUT = '''
- public_key: 87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8
	name: TEST
- address: TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y
	name: SYMBOL
'''.replace('\t', '  ')


class SymbolFacadeTest(unittest.TestCase):
	# region constants

	def test_bip32_constants_are_correct(self):
		self.assertEqual(4343, SymbolFacade.BIP32_COIN_ID)
		self.assertEqual('ed25519', SymbolFacade.BIP32_CURVE_NAME)

	def test_key_pair_is_correct(self):
		# Arrange:
		private_key = PrivateKey('E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E')

		# Act:
		key_pair = SymbolFacade.KeyPair(private_key)

		# Assert:
		self.assertEqual(PublicKey('E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD'), key_pair.public_key)

	def test_can_sign_and_verify(self):
		# Arrange:
		private_key = PrivateKey.random()
		key_pair = SymbolFacade.KeyPair(private_key)
		message = TestUtils.randbytes(21)

		# Act:
		signature = key_pair.sign(message)
		is_verified = SymbolFacade.Verifier(key_pair.public_key).verify(message, signature)

		# Assert:
		self.assertTrue(is_verified)

	# endregion

	# region constructor

	def test_can_create_around_known_network(self):
		# Act:
		facade = SymbolFacade('testnet')
		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction',
			'signer_public_key': TestUtils.random_byte_array(PublicKey)
		})

		# Assert:
		self.assertEqual('testnet', facade.network.name)
		self.assertEqual(0x98, transaction.network.value)

		self.assertEqual(0x4154, transaction.type_.value)
		self.assertEqual(1, transaction.version)

	def test_cannot_create_around_unknown_network(self):
		with self.assertRaises(StopIteration):
			SymbolFacade('foo')

	def test_can_create_via_repository(self):
		# Act:
		facade = SymbolFacade('testnet', AccountDescriptorRepository(YAML_INPUT))
		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction',
			'signer_public_key': 'TEST',
			'recipient_address': 'SYMBOL'
		})

		# Assert:
		self.assertEqual('testnet', facade.network.name)
		self.assertEqual(0x98, transaction.network.value)

		self.assertEqual(0x4154, transaction.type_.value)
		self.assertEqual(1, transaction.version)

		self.assertEqual(
			PublicKey('87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8').bytes,
			transaction.signer_public_key.bytes)
		self.assertEqual(SymbolFacade.Address('TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y').bytes, transaction.recipient_address.bytes)

	# endregion

	# region hash_transaction / sign_transaction

	@staticmethod
	def _create_real_transfer(facade):
		return facade.transaction_factory.create({
			'type': 'transfer_transaction',
			'signer_public_key': 'TEST',
			'fee': 1000000,
			'deadline': 41998024783,
			'recipient_address': 'TD4PJKW5JP3CNHA47VDFIM25RCWTWRGT45HMPSA',
			'mosaics': [
				{'mosaic_id': 0x2CF403E85507F39E, 'amount': 1000000}
			]
		})

	@staticmethod
	def _create_real_aggregate(facade):
		aggregate = facade.transaction_factory.create({
			'type': 'aggregate_complete_transaction',
			'signer_public_key': 'TEST',
			'fee': 2000000,
			'deadline': 42238390163,
			'transactions_hash': '71554638F578358B1D3FC4369AC625DB491AD5E5D4424D6DBED9FFC7411A37FE'
		})
		transfer = facade.transaction_factory.create_embedded({
			'type': 'transfer_transaction',
			'signer_public_key': 'TEST',
			'recipient_address': 'TCIDK4CGCHGVZHLNTOKJ32MFEZWMFBCWUJIAXCA',
			'mosaics': [
				{'mosaic_id': 0x2CF403E85507F39E, 'amount': 1000000}
			]
		})
		aggregate.transactions.append(transfer)
		return aggregate

	def _assert_can_hash_transaction(self, transaction_factory, expected_hash):
		# Arrange:
		private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
		facade = SymbolFacade('testnet', AccountDescriptorRepository(YAML_INPUT))

		transaction = transaction_factory(facade)
		signature = facade.sign_transaction(SymbolFacade.KeyPair(private_key), transaction)
		facade.transaction_factory.attach_signature(transaction, signature)

		# Act:
		hash_value = facade.hash_transaction(transaction)

		# Assert:
		self.assertEqual(expected_hash, hash_value)

	def test_can_hash_transaction(self):
		self._assert_can_hash_transaction(
			self._create_real_transfer,
			Hash256('17EBC7D64F01AA12F55A2B1F50C99B02BC25D06928CEAD1F249A4373B5EB1914'))

	def test_can_hash_aggregate_transaction(self):
		self._assert_can_hash_transaction(
			self._create_real_aggregate,
			Hash256('A029FCAC4957C6531B4492F08C211CDDE52C3CD72F2016D6EA37EC96B85606E7'))

	def _assert_can_sign_transaction(self, transaction_factory, expected_signature):
		# Arrange:
		private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
		facade = SymbolFacade('testnet', AccountDescriptorRepository(YAML_INPUT))

		transaction = transaction_factory(facade)

		# Sanity:
		self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

		# Act:
		signature = facade.sign_transaction(SymbolFacade.KeyPair(private_key), transaction)

		# Assert:
		self.assertEqual(expected_signature, signature)

	def test_can_sign_transaction(self):
		self._assert_can_sign_transaction(self._create_real_transfer, Signature(''.join([
			'9BC2691B3176149D5E76ED15D83BAB7AC403C754106DFA94E4264F73B92DEC1B',
			'1D514F23C07735EF394DA005AD96C86011EDF49F1FEE56CF3E280B49BEE26608'
		])))

	def test_can_sign_aggregate_transaction(self):
		self._assert_can_sign_transaction(self._create_real_aggregate, Signature(''.join([
			'CD95F7D677A66E980B0B24605049CF405CB1E350ACF65F2BC5427BBBFF531557',
			'487176A464DA6E5D6B17D71ADDD727C3D0C469513C1AB36F27547ED6101B4809'
		])))

	def _assert_can_verify_transaction(self, transaction_factory):
		# Arrange:
		private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
		facade = SymbolFacade('testnet', AccountDescriptorRepository(YAML_INPUT))

		transaction = transaction_factory(facade)

		# Sanity:
		self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

		# Act:
		signature = facade.sign_transaction(SymbolFacade.KeyPair(private_key), transaction)
		is_verified = facade.verify_transaction(transaction, signature)

		# Assert:
		self.assertTrue(is_verified)

	def test_can_verify_transaction(self):
		self._assert_can_verify_transaction(self._create_real_transfer)

	def test_can_verify_aggregate_transaction(self):
		self._assert_can_verify_transaction(self._create_real_aggregate)

	# endregion

	# region bip32_node_to_key_pair

	def _assert_bip32_child_public_keys(self, passphrase, expected_child_public_keys):
		# Arrange:
		mnemonic_seed = ' '.join([
			'hamster', 'diagram', 'private', 'dutch', 'cause', 'delay', 'private', 'meat', 'slide', 'toddler', 'razor', 'book',
			'happy', 'fancy', 'gospel', 'tennis', 'maple', 'dilemma', 'loan', 'word', 'shrug', 'inflict', 'delay', 'length'
		])

		# Act:
		root_node = Bip32(SymbolFacade.BIP32_CURVE_NAME).from_mnemonic(mnemonic_seed, passphrase)

		child_public_keys = []
		for i in range(0, len(expected_child_public_keys)):
			child_node = root_node.derive_path([44, SymbolFacade.BIP32_COIN_ID, i, 0, 0])
			child_key_pair = SymbolFacade.bip32_node_to_key_pair(child_node)
			child_public_keys.append(child_key_pair.public_key)

		# Assert:
		self.assertEqual(expected_child_public_keys, child_public_keys)

	def test_can_use_bip32_derivation_without_passphrase(self):
		self._assert_bip32_child_public_keys('', [
			PublicKey('E9CFE9F59CB4393E61B2F42769D9084A644B16883C32C2823E7DF9A3AF83C121'),
			PublicKey('0DE8C3235271E4C9ACF5482F7DFEC1E5C4B20FFC71548703EACF593153F116F9'),
			PublicKey('259866A68A00C325713342232056333D60710E223FC920566B3248B266E899D5')
		])

	def test_can_use_bip32_derivation_with_passphrase(self):
		self._assert_bip32_child_public_keys('TREZOR', [
			PublicKey('47F4D39D36D11C07735D7BE99220696AAEE7B3EE161D61422220DFE3FF120B15'),
			PublicKey('4BA67E87E8C14F3EB82B3677EA959B56A9D7355705019CED1FCF6C76104E628C'),
			PublicKey('8115D75C13C2D25E7FA3009D03D63F1F32601CDCCA9244D5FDAC74BCF3E892E3')
		])

	# endregion
