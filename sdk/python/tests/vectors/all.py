import argparse
import json
import os
import sys
import time
from binascii import unhexlify

from symbolchain.Bip32 import Bip32
from symbolchain.Cipher import AesCbcCipher, AesGcmCipher
from symbolchain.CryptoTypes import PrivateKey, PublicKey, SharedKey256, Signature
from symbolchain.Network import NetworkLocator
from symbolchain.symbol.IdGenerator import generate_mosaic_id
from symbolchain.symbol.VotingKeysGenerator import VotingKeysGenerator


class ClassLocator:
	def __init__(self, facade_class, network_class):
		self.facade_class = facade_class
		self.key_pair_class = self.facade_class.KeyPair
		self.verifier_class = self.facade_class.Verifier
		self.network_class = network_class
		self.address_class = self.facade_class.Address
		self.shared_key_class = self.facade_class.SharedKey

	@property
	def bip32_root_node_factory(self):
		return Bip32(self.facade_class.BIP32_CURVE_NAME)


# region VectorsTestSuite, KeyConversionTester, AddressConversionTester

class VectorsTestSuite:
	def __init__(self, identifier, filename, description):
		self.identifier = identifier
		self._filename = filename
		self.description = description

	@property
	def filename(self):
		return f'{self.identifier}.{self._filename}'


class KeyConversionTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(1, 'test-keys', 'key conversion')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		private_key = PrivateKey(test_vector['privateKey'])
		expected_public_key = PublicKey(test_vector['publicKey'])

		# Act:
		actual_public_key = self.class_locator.key_pair_class(private_key).public_key

		# Assert:
		return [(expected_public_key, actual_public_key)]


class AddressConversionTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(1, 'test-address', 'address conversion')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		public_key = PublicKey(test_vector['publicKey'])
		expected_address_mainnet = self.class_locator.address_class(test_vector['address_Public'])
		expected_address_testnet = self.class_locator.address_class(test_vector['address_PublicTest'])

		mainnet = NetworkLocator.find_by_name(self.class_locator.network_class.NETWORKS, 'mainnet')
		testnet = NetworkLocator.find_by_name(self.class_locator.network_class.NETWORKS, 'testnet')

		# Act:
		actual_address_mainnet = mainnet.public_key_to_address(public_key)
		actual_address_testnet = testnet.public_key_to_address(public_key)

		# Assert:
		return [
			(expected_address_mainnet, actual_address_mainnet),
			(expected_address_testnet, actual_address_testnet)
		]

# endregion


# region SignTester, VerifyTester

class SignTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(2, 'test-sign', 'sign')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		private_key = PrivateKey(test_vector['privateKey'])
		message = unhexlify(test_vector['data'])
		expected_signature = Signature(test_vector['signature'])

		# Act:
		actual_signature = self.class_locator.key_pair_class(private_key).sign(message)

		# Assert:
		return [(expected_signature, actual_signature)]


class VerifyTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(2, 'test-sign', 'verify')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		public_key = PublicKey(test_vector['publicKey'])
		message = unhexlify(test_vector['data'])
		signature = Signature(test_vector['signature'])

		# Act:
		is_verified = self.class_locator.verifier_class(public_key).verify(message, signature)

		# Assert:
		return [(True, is_verified)]

# endregion


# region DeriveDeprecatedTester, DeriveTester, CipherDeprecatedTester, CipherTester

class DeriveDeprecatedTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(3, 'test-derive-deprecated', 'derive-deprecated')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		other_public_key = PublicKey(test_vector['otherPublicKey'])
		key_pair = self.class_locator.key_pair_class(PrivateKey(test_vector['privateKey']))
		salt = unhexlify(test_vector['salt'])

		# Act:
		shared_key = self.class_locator.shared_key_class.derive_shared_key_deprecated(key_pair, other_public_key, salt)

		# Assert:
		return [(SharedKey256(test_vector['sharedKey']), shared_key)]


class DeriveTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(3, 'test-derive-hkdf', 'derive')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		other_public_key = PublicKey(test_vector['otherPublicKey'])
		key_pair = self.class_locator.key_pair_class(PrivateKey(test_vector['privateKey']))

		# Act:
		shared_key = self.class_locator.shared_key_class.derive_shared_key(key_pair, other_public_key)

		# Assert:
		return [(SharedKey256(test_vector['sharedKey']), shared_key)]


class CipherDeprecatedTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(4, 'test-cipher-deprecated', 'cipher-deprecated')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		other_public_key = PublicKey(test_vector['otherPublicKey'])
		key_pair = self.class_locator.key_pair_class(PrivateKey(test_vector['privateKey']))
		salt = unhexlify(test_vector['salt'])
		shared_key = self.class_locator.shared_key_class.derive_shared_key_deprecated(key_pair, other_public_key, salt)

		iv = unhexlify(test_vector['iv'])  # pylint: disable=invalid-name
		cipher_text = unhexlify(test_vector['cipherText'])
		clear_text = unhexlify(test_vector['clearText'])

		# Act:
		cipher = AesCbcCipher(shared_key)
		result_cipher_text = cipher.encrypt(clear_text, iv)
		result_clear_text = cipher.decrypt(cipher_text, iv)

		# Assert:
		return [
			(cipher_text, result_cipher_text),
			(clear_text, result_clear_text)
		]


class CipherTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(4, 'test-cipher', 'cipher')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		other_public_key = PublicKey(test_vector['otherPublicKey'])
		key_pair = self.class_locator.key_pair_class(PrivateKey(test_vector['privateKey']))
		shared_key = self.class_locator.shared_key_class.derive_shared_key(key_pair, other_public_key)

		iv = unhexlify(test_vector['iv'])  # pylint: disable=invalid-name
		tag = unhexlify(test_vector['tag'])
		cipher_text = unhexlify(test_vector['cipherText'])
		clear_text = unhexlify(test_vector['clearText'])

		# Act:
		cipher = AesGcmCipher(shared_key)
		result_cipher_text = cipher.encrypt(clear_text, iv)
		result_clear_text = cipher.decrypt(cipher_text + tag, iv)

		# Assert:
		return [
			(cipher_text + tag, result_cipher_text),
			(clear_text, result_clear_text)
		]

# endregion


# region MosaicIdDerivationTester

class MosaicIdDerivationTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(5, 'test-mosaic-id', 'mosaic id derivation')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		nonce = test_vector['mosaicNonce']

		mosaic_id_pairs = []
		for network_tag in ['Public', 'PublicTest', 'Private', 'PrivateTest']:
			address = self.class_locator.address_class(test_vector[f'address_{network_tag}'])
			expected_mosaic_id = test_vector[f'mosaicId_{network_tag}']

			# Act:
			mosaic_id = generate_mosaic_id(address, nonce)

			# Assert:
			mosaic_id_pairs.append((expected_mosaic_id, f'{mosaic_id:016X}'))

		return mosaic_id_pairs

# endregion


# region Bip32DerivationTester, Bip39DerivationTester

class Bip32DerivationTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(6, 'test-hd-derivation', 'BIP32 derivation')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		# Arrange:
		seed = unhexlify(test_vector['seed'])
		expected_root_public_key = PublicKey(test_vector['rootPublicKey'])

		expected_child_public_keys = []
		for child_test_vector in test_vector['childAccounts']:
			expected_child_public_keys.append(PublicKey(child_test_vector['publicKey']))

		# Act:
		root_node = self.class_locator.bip32_root_node_factory.from_seed(seed)
		root_public_key = self.class_locator.key_pair_class(root_node.private_key).public_key

		child_public_keys = []
		for child_test_vector in test_vector['childAccounts']:
			child_node = root_node.derive_path(child_test_vector['path'])
			child_key_pair = self.class_locator.facade_class.bip32_node_to_key_pair(child_node)
			child_public_keys.append(child_key_pair.public_key)

		# Assert:
		return [
			(expected_root_public_key, root_public_key),
			(expected_child_public_keys, child_public_keys)
		]


class Bip39DerivationTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(6, 'test-hd-derivation', 'BIP39 derivation')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		if 'mnemonic' not in test_vector:
			return None

		# Arrange:
		mnemonic = test_vector['mnemonic']
		passphrase = test_vector['passphrase']
		expected_root_public_key = PublicKey(test_vector['rootPublicKey'])

		# Act:
		root_node = self.class_locator.bip32_root_node_factory.from_mnemonic(mnemonic, passphrase)
		root_public_key = self.class_locator.key_pair_class(root_node.private_key).public_key

		# Assert:
		return [(expected_root_public_key, root_public_key)]

# endregion


# region VotingKeysGenerationTester

class SeededPrivateKeyGenerator:
	def __init__(self, values):
		self.values = values
		self.next_index = 0

	def generate(self):
		self.next_index += 1
		return self.values[self.next_index - 1]


class FibPrivateKeyGenerator:
	def __init__(self, fill_private_key=False):
		self.fill_private_key = fill_private_key
		self.value1 = 1
		self.value2 = 2

	def generate(self):
		next_value = self.value1 + self.value2
		self.value1 = self.value2
		self.value2 = next_value

		seed_value = next_value % 256

		if not self.fill_private_key:
			return PrivateKey(seed_value.to_bytes(PrivateKey.SIZE, 'big'))

		return PrivateKey(bytes([(seed_value + i) % 256 for i in range(0, PrivateKey.SIZE)]))


class VotingKeysGenerationTester(VectorsTestSuite):
	def __init__(self, class_locator):
		super().__init__(7, 'test-voting-keys-generation', 'voting keys generation')
		self.class_locator = class_locator

	def process(self, test_vector, _):
		private_key_generator = {
			'test_vector_1': FibPrivateKeyGenerator(),
			'test_vector_2': FibPrivateKeyGenerator(True),
			'test_vector_3': SeededPrivateKeyGenerator([
				PrivateKey('12F98B7CB64A6D840931A2B624FB1EACAFA2C25C3EF0018CD67E8D470A248B2F'),
				PrivateKey('B5593870940F28DAEE262B26367B69143AD85E43048D23E624F4ED8008C0427F'),
				PrivateKey('6CFC879ABCCA78F5A4C9739852C7C643AEC3990E93BF4C6F685EB58224B16A59')
			])
		}[test_vector['name']]

		root_private_key = PrivateKey(test_vector['rootPrivateKey'])
		voting_keys_generator = VotingKeysGenerator(self.class_locator.key_pair_class(root_private_key), private_key_generator.generate)

		# Act:
		voting_keys_buffer = voting_keys_generator.generate(test_vector['startEpoch'], test_vector['endEpoch'])

		# Assert:
		expected_voting_keys_buffer = unhexlify(test_vector['expectedFileHex'])
		return [(expected_voting_keys_buffer, voting_keys_buffer)]

# endregion


def load_class_locator(blockchain):
	# pylint: disable=import-outside-toplevel

	if 'symbol' == blockchain:
		from symbolchain.facade.SymbolFacade import SymbolFacade
		from symbolchain.symbol.Network import Network
		return ClassLocator(SymbolFacade, Network)

	from symbolchain.facade.NemFacade import NemFacade
	from symbolchain.nem.Network import Network
	return ClassLocator(NemFacade, Network)


def load_test_suites(blockchain):
	class_locator = load_class_locator(blockchain)
	test_suites = [
		KeyConversionTester(class_locator),
		AddressConversionTester(class_locator),
		SignTester(class_locator),
		VerifyTester(class_locator),
		DeriveTester(class_locator),
		Bip32DerivationTester(class_locator),
		Bip39DerivationTester(class_locator),
		CipherTester(class_locator)
	]

	if 'symbol' == blockchain:
		test_suites += [MosaicIdDerivationTester(class_locator), VotingKeysGenerationTester(class_locator)]
	else:
		test_suites += [DeriveDeprecatedTester(class_locator), CipherDeprecatedTester(class_locator)]

	return test_suites


def main():
	# pylint: disable=too-many-locals

	test_identifiers = range(0, 8)
	parser = argparse.ArgumentParser(description='nem test vectors harness')
	parser.add_argument('--vectors', help='path to test-vectors directory', required=True)
	parser.add_argument('--blockchain', help='blockchain to run vectors against', choices=['nem', 'symbol'], default='symbol')
	parser.add_argument(
		'--tests',
		help='identifiers of tests to include',
		type=int,
		nargs='+',
		choices=test_identifiers,
		default=test_identifiers)
	args = parser.parse_args()

	print(f'running tests for {args.blockchain} blockchain with vectors from {args.vectors}')

	num_failed_suites = 0
	test_suites = load_test_suites(args.blockchain)
	for test_suite in test_suites:
		if test_suite.identifier not in args.tests:
			print(f'[ SKIPPED ] {test_suite.description} test')
			continue

		with open(os.path.join(args.vectors, f'{test_suite.filename}.json'), 'rt', encoding='utf8') as infile:
			start_time = time.time()

			test_case_number = 0
			num_failed = 0

			parsed_json = json.loads(infile.read())
			for test_cases in parsed_json:
				if isinstance(test_cases, str):
					test_group_name = test_cases
					test_cases = parsed_json[test_group_name]
				else:
					test_group_name = None
					test_cases = [test_cases]

				for test_case in test_cases:
					processed_pairs = test_suite.process(test_case, test_group_name)
					if None is processed_pairs:
						continue

					if any(pair[0] != pair[1] for pair in processed_pairs):
						num_failed += 1

					test_case_number += 1

			elapsed_time = time.time() - start_time

			test_message_prefix = f'[{elapsed_time:8.4f}s] {test_suite.description} test:'
			if num_failed:
				print(f'{test_message_prefix} {num_failed} failures out of {test_case_number}')
				num_failed_suites += 1
			else:
				print(f'{test_message_prefix} successes {test_case_number}')

	if num_failed_suites:
		sys.exit(1)


if '__main__' == __name__:
	main()
