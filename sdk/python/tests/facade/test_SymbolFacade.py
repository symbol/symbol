import unittest

from symbolchain import sc
from symbolchain.AccountDescriptorRepository import AccountDescriptorRepository
from symbolchain.Bip32 import Bip32
from symbolchain.CryptoTypes import Hash256, PrivateKey, PublicKey, Signature
from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.Network import Network

from ..test.TestUtils import TestUtils

YAML_INPUT = '''
- public_key: 87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8
	name: TEST
- address: TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y
	name: SYMBOL
'''.replace('\t', '  ')


class SymbolFacadeTest(unittest.TestCase):
	# pylint: disable=too-many-public-methods

	# region real transactions

	@staticmethod
	def _create_real_transfer(facade):
		return facade.transaction_factory.create({
			'type': 'transfer_transaction_v1',
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
			'type': 'aggregate_complete_transaction_v1',
			'signer_public_key': 'TEST',
			'fee': 2000000,
			'deadline': 42238390163,
			'transactions_hash': '71554638F578358B1D3FC4369AC625DB491AD5E5D4424D6DBED9FFC7411A37FE'
		})
		transfer = facade.transaction_factory.create_embedded({
			'type': 'transfer_transaction_v1',
			'signer_public_key': 'TEST',
			'recipient_address': 'TCIDK4CGCHGVZHLNTOKJ32MFEZWMFBCWUJIAXCA',
			'mosaics': [
				{'mosaic_id': 0x2CF403E85507F39E, 'amount': 1000000}
			]
		})
		aggregate.transactions.append(transfer)
		return aggregate

	@staticmethod
	def _create_real_embedded_transactions(facade):
		return list(map(facade.transaction_factory.create_embedded, [
			{
				'type': 'transfer_transaction_v1',
				'signer_public_key': 'TEST',
				'recipient_address': 'TCIDK4CGCHGVZHLNTOKJ32MFEZWMFBCWUJIAXCA',
				'mosaics': [
					{'mosaic_id': 0x2CF403E85507F39E, 'amount': 1000000}
				]
			},
			{
				'type': 'secret_proof_transaction_v1',
				'signer_public_key': 'TEST',
				'recipient_address': 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y',
				'secret': 'BE254D2744329BBE20F9CF6DA61043B4CEF8C2BC000000000000000000000000',
				'hash_algorithm': 'hash_256',
				'proof': '41FB'
			},
			{
				'type': 'address_alias_transaction_v1',
				'signer_public_key': 'TEST',
				'namespace_id': 0xA95F1F8A96159516,
				'address': 'TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y',
				'alias_action': 'link'
			}
		]))

	@staticmethod
	def _create_real_aggregate_swap(facade):
		return facade.transaction_factory.create({
			'type': 'aggregate_complete_transaction_v1',
			'signer_public_key': '4C94E8B0A1DAB8573BCB6632E676F742E0D320FC8102F20FB7FB13BCAE9A9F60',
			'fee': 36000,
			'deadline': 26443750218,
			'transactions_hash': '641CB7E431F1D44094A43E1CE8265E6BD1DF1C3B0B64797CDDAA0A375FCD3C08',
			'transactions': [
				facade.transaction_factory.create_embedded({
					'type': 'transfer_transaction_v1',
					'signer_public_key': '29856F43A5C4CBDE42F2FAC775A6F915E9E5638CF458E9352E7B410B662473A3',
					'recipient_address': 'TBEZ3VKFBMKQSW7APBVL5NWNBEU7RR466PRRTDQ',
					'mosaics': [
						{'mosaic_id': 0xE74B99BA41F4AFEE, 'amount': 20000000}
					]
				}),
				facade.transaction_factory.create_embedded({
					'type': 'transfer_transaction_v1',
					'signer_public_key': '4C94E8B0A1DAB8573BCB6632E676F742E0D320FC8102F20FB7FB13BCAE9A9F60',
					'recipient_address': 'TDFR3Q3H5W4OPOSHALVDY3RF4ZQNH44LIUIHYTQ',
					'mosaics': [
						{'mosaic_id': 0x798A29F48E927C83, 'amount': 100}
					]
				})
			]
		})

	# endregion

	# region constants

	def test_bip32_constants_are_correct(self):
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

	def test_can_create_around_known_network_by_name(self):
		# Act:
		facade = SymbolFacade('testnet')
		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction_v1',
			'signer_public_key': bytes(PublicKey.SIZE)
		})

		# Assert:
		self.assertEqual('testnet', facade.network.name)
		self.assertEqual(0x98, transaction.network.value)

		self.assertEqual(0x4154, transaction.type_.value)
		self.assertEqual(1, transaction.version)

	def test_cannot_create_around_unknown_network_by_name(self):
		with self.assertRaises(StopIteration):
			SymbolFacade('foo')

	def test_can_create_around_unknown_network(self):
		# Arrange:
		network = Network('foo', 0x98, None)

		# Act:
		facade = SymbolFacade(network)
		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction_v1',
			'signer_public_key': bytes(PublicKey.SIZE)
		})

		# Assert:
		self.assertEqual('foo', facade.network.name)
		self.assertEqual(0x98, transaction.network.value)

		self.assertEqual(0x4154, transaction.type_.value)
		self.assertEqual(1, transaction.version)

	def test_can_create_via_repository(self):
		# Act:
		facade = SymbolFacade('testnet', AccountDescriptorRepository(YAML_INPUT))
		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction_v1',
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
			Hash256('86E006F0D400A781A15D0293DFC15897078351A2F7731D49A865A63C2010DE44'))

	def test_can_hash_aggregate_transaction(self):
		self._assert_can_hash_transaction(
			self._create_real_aggregate,
			Hash256('D074716D62F4CDF1CE219D7E0580DC2C030102E216ECE2037FA28A3BC5726BD0'))

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
			'24A3788AFD0223083D47ED14F17A2499A7939CD62C4B3288C40CF2736B13F404',
			'8486680DD574C9F7DB56F453464058CB22349ACBFAECAE16A31EF0725FFF6104'
		])))

	def test_can_sign_aggregate_transaction(self):
		self._assert_can_sign_transaction(self._create_real_aggregate, Signature(''.join([
			'40C5C9F0BAF74E64877982C411D0D16665E18D463B66204081D846564FC6CAE1',
			'3F1F75C688CBD2D34263DA166537A90B4F371C1B38DDF00414AB0F5D78C3CD0F'
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

	# region cosign_transaction

	def _assert_can_cosign_transaction(self, detached=False):
		# Arrange:
		signer_private_key = PrivateKey('F4BC233E183E8CEA08D0A604A3DC67FF3261D1E6EBF84D233488BC53D89C50B7')
		cosigner_private_key = PrivateKey('BE7B98F835A896136ADDAF04220F28CB4925D24F0675A21421BF213C180BEF86')
		facade = SymbolFacade('testnet', AccountDescriptorRepository(YAML_INPUT))

		transaction = self._create_real_aggregate_swap(facade)
		signature = facade.sign_transaction(SymbolFacade.KeyPair(signer_private_key), transaction)
		facade.transaction_factory.attach_signature(transaction, signature)

		# Act:
		cosignature = facade.cosign_transaction(SymbolFacade.KeyPair(cosigner_private_key), transaction, detached)

		# Assert: check common fields
		self.assertEqual(0, cosignature.version)
		self.assertEqual(sc.PublicKey('29856F43A5C4CBDE42F2FAC775A6F915E9E5638CF458E9352E7B410B662473A3'), cosignature.signer_public_key)
		self.assertEqual(
			sc.Signature('204BD2C4F86B66313E5C5F817FD650B108826D53EDEFC8BDFF936E4D6AA07E38' + (
				'5F819CF0BF22D14D4AA2011AD07BC0FE6023E2CB48DC5D82A6A1FF1348FA3E0B'
			)),
			cosignature.signature)
		return cosignature

	def test_can_cosign_transaction(self):
		# Act:
		cosignature = self._assert_can_cosign_transaction()

		# Assert: cosignature should be suitable for attaching to an aggregate
		self.assertEqual(104, cosignature.size)
		self.assertFalse(hasattr(cosignature, 'parent_hash'))

	def test_can_cosign_transaction_detached(self):
		# Act:
		cosignature = self._assert_can_cosign_transaction(True)

		# Assert: cosignature should be detached
		self.assertEqual(136, cosignature.size)
		self.assertEqual(sc.Hash256('214DFF47469D462E1D9A03232C2582C7E44DE026A287F98529CC74DE9BD69641'), cosignature.parent_hash)

	# endregion

	# region hash_embedded_transactions

	def test_can_hash_embedded_transactions(self):
		# Arrange:
		facade = SymbolFacade('testnet', AccountDescriptorRepository(YAML_INPUT))
		transaction = self._create_real_aggregate(facade)

		# Act:
		hash_value = facade.hash_embedded_transactions(transaction.transactions)

		# Assert:
		self.assertEqual(transaction.transactions_hash.bytes, hash_value.bytes)

	def test_can_hash_embedded_transactions_multiple(self):
		# Arrange:
		facade = SymbolFacade('testnet', AccountDescriptorRepository(YAML_INPUT))
		transactions = self._create_real_embedded_transactions(facade)

		# Act:
		hash_value = facade.hash_embedded_transactions(transactions)

		# Assert:
		self.assertEqual(Hash256('5C78999F21EA75B880100E1B4C76166B9C320869F67C00D28F9F8F754D7831C9'), hash_value)

	# endregion

	# region bip32_path

	def test_can_construct_proper_bip32_mainnet_path(self):
		# Arrange:
		facade = SymbolFacade('mainnet')

		# Act:
		path = facade.bip32_path(2)

		# Act + Assert:
		self.assertEqual([44, 4343, 2, 0, 0], path)

	def test_can_construct_proper_bip32_testnet_path(self):
		# Arrange:
		facade = SymbolFacade('testnet')

		# Act:
		path = facade.bip32_path(2)

		# Act + Assert:
		self.assertEqual([44, 1, 2, 0, 0], path)

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
			child_node = root_node.derive_path(SymbolFacade('mainnet').bip32_path(i))
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
