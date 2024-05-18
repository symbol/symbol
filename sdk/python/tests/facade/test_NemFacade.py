import unittest
from datetime import datetime, timezone

from symbolchain.AccountDescriptorRepository import AccountDescriptorRepository
from symbolchain.Bip32 import Bip32
from symbolchain.CryptoTypes import Hash256, PrivateKey, PublicKey, Signature
from symbolchain.facade.NemFacade import NemFacade
from symbolchain.nem.Network import Network

from ..test.TestUtils import TestUtils

YAML_INPUT = '''
- public_key: A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74
	name: TEST
- address: TALIC33PNVKIMNXVOCOQGWLZK52K4XALZBNE2ISF
	name: ALICE
'''.replace('\t', '  ')


class NemFacadeTest(unittest.TestCase):
	# pylint: disable=too-many-public-methods

	# region real transactions

	@staticmethod
	def _create_real_transfer():
		facade = NemFacade('testnet', AccountDescriptorRepository(YAML_INPUT))
		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction_v1',
			'signer_public_key': 'TEST',
			'fee': 0x186A0,
			'timestamp': 191205516,
			'deadline': 191291916,
			'recipient_address': 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
			'amount': 5100000,
			'message': {
				'message_type': 'plain',
				'message': 'blah blah'
			}
		})
		return transaction

	@staticmethod
	def _create_real_multisig_transaction():
		facade = NemFacade('testnet', AccountDescriptorRepository(YAML_INPUT))
		factory = facade.transaction_factory

		inner_transaction = factory.to_non_verifiable_transaction(NemFacadeTest._create_real_transfer())
		transaction = facade.transaction_factory.create({
			'type': 'multisig_transaction_v1',
			'signer_public_key': 'TEST',
			'fee': 0x123456,
			'timestamp': 191205516,
			'deadline': 191291916,

			'inner_transaction': inner_transaction,
		})
		return transaction

	# endregion

	# region constants

	def test_bip32_constants_are_correct(self):
		self.assertEqual('ed25519-keccak', NemFacade.BIP32_CURVE_NAME)

	def test_key_pair_is_correct(self):
		# Arrange:
		private_key = PrivateKey('ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57')  # reversed

		# Act:
		key_pair = NemFacade.KeyPair(private_key)

		# Assert:
		self.assertEqual(PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0'), key_pair.public_key)

	def test_can_sign_and_verify(self):
		# Arrange:
		private_key = PrivateKey.random()
		key_pair = NemFacade.KeyPair(private_key)
		message = TestUtils.randbytes(21)

		# Act:
		signature = key_pair.sign(message)
		is_verified = NemFacade.Verifier(key_pair.public_key).verify(message, signature)

		# Assert:
		self.assertTrue(is_verified)

	# endregion

	# region constructor

	def test_can_create_around_known_network_by_name(self):
		# Act:
		facade = NemFacade('testnet')
		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction_v2',
			'signer_public_key': bytes(PublicKey.SIZE)
		})

		# Assert:
		self.assertEqual('testnet', facade.network.name)
		self.assertEqual(0x98, transaction.network.value)

		self.assertEqual(0x0101, transaction.type_.value)
		self.assertEqual(2, transaction.version)

	def test_cannot_create_around_unknown_network_by_name(self):
		with self.assertRaises(StopIteration):
			NemFacade('foo')

	def test_can_create_around_unknown_network(self):
		# Arrange:
		network = Network('foo', 0x98, None)

		# Act:
		facade = NemFacade(network)
		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction_v2',
			'signer_public_key': bytes(PublicKey.SIZE)
		})

		# Assert:
		self.assertEqual('foo', facade.network.name)
		self.assertEqual(0x98, transaction.network.value)

		self.assertEqual(0x0101, transaction.type_.value)
		self.assertEqual(2, transaction.version)

	def test_can_create_via_repository(self):
		# Act:
		facade = NemFacade('testnet', AccountDescriptorRepository(YAML_INPUT))
		transaction = facade.transaction_factory.create({
			'type': 'transfer_transaction_v2',
			'signer_public_key': 'TEST',
			'recipient_address': 'ALICE'
		})

		# Assert:
		self.assertEqual('testnet', facade.network.name)
		self.assertEqual(0x98, transaction.network.value)

		self.assertEqual(0x0101, transaction.type_.value)
		self.assertEqual(2, transaction.version)

		self.assertEqual(
			PublicKey('A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74').bytes,
			transaction.signer_public_key.bytes)
		self.assertEqual(
			str(NemFacade.Address('TALIC33PNVKIMNXVOCOQGWLZK52K4XALZBNE2ISF')).encode('utf8'),
			transaction.recipient_address.bytes)

	# endregion

	# region now

	def test_can_create_current_timestamp_for_network_via_now(self):
		while True:
			# Arrange: affinitize test to run so that whole test runs within the context of the same millisecond
			start_time = datetime.now()
			facade = NemFacade('testnet')

			# Act:
			now_from_facade = facade.now()
			now_from_network = facade.network.from_datetime(datetime.now(timezone.utc))

			end_time = datetime.now()
			if (start_time.microsecond // 1000) != (end_time.microsecond // 1000):
				continue

			# Assert:
			self.assertEqual(now_from_network, now_from_facade)
			self.assertGreater(now_from_facade.timestamp, 0)
			break

	# endregion

	# region create_public_account / create_account

	def test_can_create_public_account_from_public_key(self):
		# Arrange:
		facade = NemFacade('testnet')
		public_key = PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0')

		# Act:
		account = facade.create_public_account(public_key)

		# Assert:
		self.assertEqual(facade.Address('TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33'), account.address)
		self.assertEqual(public_key, account.public_key)

	def test_can_create_account_from_private_key(self):
		# Arrange:
		facade = NemFacade('testnet')
		public_key = PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0')
		private_key = PrivateKey('ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57')

		# Act:
		account = facade.create_account(private_key)

		# Assert:
		self.assertEqual(facade.Address('TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33'), account.address)
		self.assertEqual(public_key, account.public_key)
		self.assertEqual(public_key, account.key_pair.public_key)
		self.assertEqual(private_key, account.key_pair.private_key)

	def test_can_sign_transaction_with_account_wrappers(self):
		# Arrange:
		facade = NemFacade('testnet', AccountDescriptorRepository(YAML_INPUT))
		account = facade.create_account(PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC'))

		transaction = self._create_real_transfer()

		# Sanity:
		self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

		# Act:
		signature = account.sign_transaction(transaction)
		is_verified = facade.verify_transaction(transaction, signature)

		# Assert:
		self.assertTrue(is_verified)

	# endregion

	# region hash_transaction / sign_transaction

	def test_can_hash_transaction(self):
		# Arrange:
		transaction = self._create_real_transfer()

		# Act:
		hash_value = NemFacade.hash_transaction(transaction)

		# Assert:
		self.assertEqual(Hash256('A7064DB890A4E7329AAB2AE7DCFA5EC76D7E374590C61EC85E03C698DF4EA79D'), hash_value)

	def test_can_sign_transaction(self):
		# Arrange:
		private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
		transaction = self._create_real_transfer()

		# Sanity:
		self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

		# Act:
		signature = NemFacade.sign_transaction(NemFacade.KeyPair(private_key), transaction)

		# Assert:
		expected_signature = Signature(''.join([
			'23A7B3433D16172E6C8659DB24233C5A8222C589098EA7A8FBBCB19691C67DB1'
			'3FB2AB7BB215265A3E3D74D32683516B03785BFEB2A2DE6DAC09F5E34A793706'
		]))
		self.assertEqual(expected_signature, signature)

	def test_can_verify_transaction(self):
		# Arrange:
		private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
		transaction = self._create_real_transfer()

		# Sanity:
		self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

		# Act:
		signature = NemFacade.sign_transaction(NemFacade.KeyPair(private_key), transaction)
		is_verified = NemFacade.verify_transaction(transaction, signature)

		# Assert:
		self.assertTrue(is_verified)

	# endregion

	# region multisig

	def test_can_hash_multisig_transaction(self):
		# Arrange:
		transaction = self._create_real_multisig_transaction()

		# Act:
		hash_value = NemFacade.hash_transaction(transaction)

		# Assert:
		self.assertEqual(Hash256('B585BC092CDDDCBA535FD6C0DE38F26EB44E6BA638A0BA6DFAD4BAA7E7AAE1B8'), hash_value)

	def test_can_sign_multisig_transaction(self):
		# Arrange:
		private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
		transaction = self._create_real_multisig_transaction()

		# Sanity:
		self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

		# Act:
		signature = NemFacade.sign_transaction(NemFacade.KeyPair(private_key), transaction)

		# Assert:
		print(signature)
		expected_signature = Signature(''.join([
			'E324CCA57275D9752A684E6A089733803423647B8DDF5C1627FC23218CC84287'
			'EB7037AD4C6CB8CB37BBC9F5423FA73F431814A008400A756CFFE35F4533EB00'
		]))
		self.assertEqual(expected_signature, signature)

	def test_can_verify_multisig_transaction(self):
		# Arrange:
		private_key = PrivateKey('EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC')
		transaction = self._create_real_multisig_transaction()

		# Sanity:
		self.assertEqual(Signature.zero().bytes, transaction.signature.bytes)

		# Act:
		signature = NemFacade.sign_transaction(NemFacade.KeyPair(private_key), transaction)
		is_verified = NemFacade.verify_transaction(transaction, signature)

		# Assert:
		self.assertTrue(is_verified)

	# endregion

	# region bip32_path

	def test_can_construct_proper_bip32_mainnet_path(self):
		# Arrange:
		facade = NemFacade('mainnet')

		# Act:
		path = facade.bip32_path(2)

		# Act + Assert:
		self.assertEqual([44, 43, 2, 0, 0], path)

	def test_can_construct_proper_bip32_testnet_path(self):
		# Arrange:
		facade = NemFacade('testnet')

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
		root_node = Bip32(NemFacade.BIP32_CURVE_NAME).from_mnemonic(mnemonic_seed, passphrase)

		child_public_keys = []
		for i in range(0, len(expected_child_public_keys)):
			child_node = root_node.derive_path(NemFacade('mainnet').bip32_path(i))
			child_key_pair = NemFacade.bip32_node_to_key_pair(child_node)
			child_public_keys.append(child_key_pair.public_key)

		# Assert:
		self.assertEqual(expected_child_public_keys, child_public_keys)

	def test_can_use_bip32_derivation_without_passphrase(self):
		self._assert_bip32_child_public_keys('', [
			PublicKey('6C42BFAD2199CCB5C64E59868CC7A3F2AD29BDDCEB9754157DF136535E6B5EBA'),
			PublicKey('782FF2375F75524106356092B4EE4BA098D28CF6571F1643867B9A11AEF509C6'),
			PublicKey('20EEEFCAE026EBB3C3C51E9AF86A97AA146B34A5463CFE468B37C3CB49682408')
		])

	def test_can_use_bip32_derivation_with_passphrase(self):
		self._assert_bip32_child_public_keys('TREZOR', [
			PublicKey('3BE4759796DD507D6E410CD8C65121E7E42DAC69699A9058E1F7663A390122CE'),
			PublicKey('6B288C00800EC9FC0C30F35CEAFC2C5EC4066C2BE622822AAC70D67F215E5E6D'),
			PublicKey('1AC159878D327E578C0130767E960C265753CAD5215FC992F1F71C41D00EADA3')
		])

	# endregion
