import os
import tempfile
import unittest

from symbolchain.AccountDescriptorRepository import AccountDescriptorRepository
from symbolchain.CryptoTypes import Hash256, PrivateKey, PublicKey, Signature
from symbolchain.facade.BatchOperations import BatchOperations
from symbolchain.facade.NemFacade import NemFacade
from symbolchain.nem.Network import Address
from symbolchain.PrivateKeyStorage import PrivateKeyStorage
from symbolchain.QrSignatureStorage import QrSignatureStorage

from ..test.TestUtils import TestUtils

# NemFacade is used in tests, so constants are using nem formats

TEST_PUBLIC_KEY = PublicKey('0F74A2F537CD9C986DF018994DDE75BDEEE05E35EB9FE27ADF506CA8475064F7')
TEST_PRIVATE_KEY = PrivateKey('D9639DC6F49DAD02A42FD8C217F1B1B4F8CE31CCD770388B645E639C72FF24FA')
ALICE_ADDRESS = Address('TALIC33PNVKIMNXVOCOQGWLZK52K4XALZBNE2ISF')
BOB_ADDRESS = Address('TBZ6JK5YOCU6UPSSZ5D3G27UHAPHTY5HDQCDS5YA')

ACCOUNTS_YAML_INPUT = f'''
- public_key: {TEST_PUBLIC_KEY}
	name: TEST

- address: {ALICE_ADDRESS}
	name: ALICE
'''.replace('\t', '  ')

TRANSACTIONS_YAML_INPUT = f'''
- type: transfer_transaction_v2
	signer_public_key: TEST
	recipient_address: ALICE
	amount: 3000000
	message:
		message_type: plain
		message: Hello world!

- type: transfer_transaction_v2
	signer_public_key: TEST
	recipient_address: {BOB_ADDRESS}
	amount: 1000000
'''.replace('\t', '  ')


class BatchOperationsTest(unittest.TestCase):
	# region load_all

	def test_can_load_all_transactions(self):
		# Arrange:
		operations = self._create_operations()

		# Act:
		transactions = operations.load_all(TRANSACTIONS_YAML_INPUT)

		# Assert:
		self.assertEqual(2, len(transactions))

		self.assertEqual(0x0101, transactions[0].type_.value)
		self.assertEqual(2, transactions[0].version)
		self.assertEqual(0x98, transactions[0].network.value)

		self.assertEqual(TEST_PUBLIC_KEY.bytes, transactions[0].signer_public_key.bytes)
		self.assertEqual(str(ALICE_ADDRESS).encode('utf8'), transactions[0].recipient_address.bytes)
		self.assertEqual(3000000, transactions[0].amount.value)
		self.assertEqual(b'Hello world!', transactions[0].message.message)

		self.assertEqual(0x0101, transactions[1].type_.value)
		self.assertEqual(2, transactions[1].version)
		self.assertEqual(0x98, transactions[1].network.value)

		self.assertEqual(TEST_PUBLIC_KEY.bytes, transactions[1].signer_public_key.bytes)
		self.assertEqual(str(BOB_ADDRESS).encode('utf8'), transactions[1].recipient_address.bytes)
		self.assertEqual(1000000, transactions[1].amount.value)
		self.assertEqual(None, transactions[1].message)

	# endregion

	# region sign_all

	def test_cannot_sign_all_transactions_when_private_keys_are_not_available(self):
		# Arrange:
		operations = self._create_operations('test')
		transactions = operations.load_all(TRANSACTIONS_YAML_INPUT)

		with tempfile.TemporaryDirectory() as temp_directory:
			(private_key_storage, signature_storage) = self._create_storages(temp_directory, False)

			# Act + Assert:
			with self.assertRaises(FileNotFoundError):
				operations.sign_all(transactions, private_key_storage, signature_storage)

	def test_can_sign_all_transactions_when_private_keys_are_available(self):
		# Arrange:
		operations = self._create_operations('test')
		transactions = operations.load_all(TRANSACTIONS_YAML_INPUT)

		with tempfile.TemporaryDirectory() as temp_directory:
			(private_key_storage, signature_storage) = self._create_storages(temp_directory)

			# Act:
			operations.sign_all(transactions, private_key_storage, signature_storage)

			# Assert:
			signature_filenames = os.listdir(os.path.join(temp_directory, 'signatures'))
			self.assertEqual(2, len(signature_filenames))
			for filename in ['sig_test0.png', 'sig_test1.png']:
				self.assertTrue(filename in signature_filenames)

	# endregion

	# region prepare_all

	def test_cannot_prepare_all_when_more_transactions_than_signature_groups(self):
		# Arrange:
		operations = self._create_operations('test')
		transactions = operations.load_all(TRANSACTIONS_YAML_INPUT)

		with tempfile.TemporaryDirectory() as temp_directory:
			(private_key_storage, signature_storage) = self._create_storages(temp_directory)
			operations.sign_all(transactions, private_key_storage, signature_storage)

			payload_directory = os.path.join(temp_directory, 'payloads')
			os.mkdir(payload_directory)

			# Act + Assert:
			with self.assertRaises(FileNotFoundError):
				operations.prepare_all(transactions + [transactions[0]], signature_storage, payload_directory)

			# Sanity: no payloads were created
			self.assertEqual(0, len(os.listdir(payload_directory)))

	def _assert_cannot_prepare_all_when_transaction_verification_fails(self, corrupt_hash=False, corrupt_signature=False):
		# Arrange:
		operations = self._create_operations('test')
		transactions = operations.load_all(TRANSACTIONS_YAML_INPUT)

		with tempfile.TemporaryDirectory() as temp_directory:
			(private_key_storage, signature_storage) = self._create_storages(temp_directory)
			operations.sign_all(transactions, private_key_storage, signature_storage)

			payload_directory = os.path.join(temp_directory, 'payloads')
			os.mkdir(payload_directory)

			# overrwrite second qrcode with the intention to corrupt it
			if corrupt_hash:
				transaction_hash = TestUtils.random_byte_array(Hash256)
			else:
				transaction_hash = operations.facade.hash_transaction(transactions[1])

			if corrupt_signature:
				signature = TestUtils.random_byte_array(Signature)
			else:
				# note: wrapping is needed due to different PublicKey types, facade requires sdk type
				signer_public_key = PublicKey(transactions[1].signer_public_key.bytes)
				signer_account_name = operations.facade.account_descriptor_repository.find_by_public_key(signer_public_key).name
				signer_private_key = private_key_storage.load(signer_account_name)
				signature = operations.facade.KeyPair(signer_private_key).sign(transactions[1].serialize())

			signature_storage.save('sig_test1', transaction_hash, [signature])

			# Act + Assert:
			with self.assertRaises(BatchOperations.PrepareError):
				operations.prepare_all(transactions, signature_storage, payload_directory)

			# Sanity: no payloads were created
			self.assertEqual(0, len(os.listdir(payload_directory)))

	def test_cannot_prepare_all_when_transaction_hash_does_not_match(self):
		self._assert_cannot_prepare_all_when_transaction_verification_fails(corrupt_hash=True)

	def test_cannot_prepare_all_when_transaction_signature_does_not_match(self):
		self._assert_cannot_prepare_all_when_transaction_verification_fails(corrupt_signature=True)

	def test_can_prepare_all_when_all_transactions_match_signatures(self):
		# Arrange:
		operations = self._create_operations('test')
		transactions = operations.load_all(TRANSACTIONS_YAML_INPUT)

		with tempfile.TemporaryDirectory() as temp_directory:
			(private_key_storage, signature_storage) = self._create_storages(temp_directory)
			operations.sign_all(transactions, private_key_storage, signature_storage)

			payload_directory = os.path.join(temp_directory, 'payloads')
			os.mkdir(payload_directory)

			# Act:
			operations.prepare_all(transactions, signature_storage, payload_directory)

			# Assert: correct size payloads were written
			payload_filenames = os.listdir(payload_directory)
			self.assertEqual(2, len(payload_filenames))
			for i, filename in enumerate(['payload_test0.json', 'payload_test1.json']):
				self.assertTrue(filename in payload_filenames)

				# note transaction.serialize includes signatures
				non_verifiable_buffer = operations.facade.transaction_factory.to_non_verifiable_transaction(transactions[i]).serialize()
				expected_file_size = 2 * len(non_verifiable_buffer) + 2 * Signature.SIZE + len('{"data":"') + len('", "signature":"') + len('"}')
				actual_file_size = os.path.getsize(os.path.join(payload_directory, filename))
				self.assertEqual(expected_file_size, actual_file_size)

	# endregion

	@staticmethod
	def _create_operations(output_file_prefix=''):
		facade = NemFacade('testnet', AccountDescriptorRepository(ACCOUNTS_YAML_INPUT))
		return BatchOperations(facade, output_file_prefix)

	@staticmethod
	def _create_storages(temp_directory, save_test_private_key=True):
		private_key_directory = os.path.join(temp_directory, 'private_keys')
		os.mkdir(private_key_directory)
		private_key_storage = PrivateKeyStorage(private_key_directory, 'password')
		if save_test_private_key:
			private_key_storage.save('TEST', TEST_PRIVATE_KEY)

		signature_directory = os.path.join(temp_directory, 'signatures')
		os.mkdir(signature_directory)
		signature_storage = QrSignatureStorage(signature_directory)

		return (private_key_storage, signature_storage)
