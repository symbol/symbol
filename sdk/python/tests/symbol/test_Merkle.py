import unittest

from symbolchain.CryptoTypes import Hash256
from symbolchain.symbol.Merkle import MerkleHashBuilder, MerklePart, prove_merkle

from ..test.TestUtils import TestUtils

# region MerkleHashBuilderTest


class MerkleHashBuilderTest(unittest.TestCase):
	def test_can_build_from_zero_hashes(self):
		# Act:
		merkle_hash = self._calculate_merkle_hash([])

		# Assert:
		self.assertEqual(Hash256.zero(), merkle_hash)

	def test_can_build_from_one_hash(self):
		# Arrange:
		seed_hash = TestUtils.random_byte_array(Hash256)

		# Act:
		merkle_hash = self._calculate_merkle_hash([seed_hash])

		# Assert:
		self.assertEqual(seed_hash, merkle_hash)

	def test_can_build_from_balanced_tree(self):
		# Act:
		merkle_hash = self._calculate_merkle_hash([
			Hash256('36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8'),
			Hash256('8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28'),
			Hash256('6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7'),
			Hash256('2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681'),
			Hash256('421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE'),
			Hash256('7A1711AF5C402CFEFF87F6DA4B9C738100A7AC3EDAD38D698DF36CA3FE883480'),
			Hash256('1E6516B2CC617E919FAE0CF8472BEB2BFF598F19C7A7A7DC260BC6715382822C'),
			Hash256('410330530D04A277A7C96C1E4F34184FDEB0FFDA63563EFD796C404D7A6E5A20')
		])

		# Assert:
		self.assertEqual(Hash256('7D853079F5F9EE30BDAE49C4956AF20CDF989647AFE971C069AC263DA1FFDF7E'), merkle_hash)

	def test_can_build_from_unbalanced_tree(self):
		# Act:
		merkle_hash = self._calculate_merkle_hash([
			Hash256('36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8'),
			Hash256('8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28'),
			Hash256('6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7'),
			Hash256('2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681'),
			Hash256('421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE')
		])

		# Assert:
		self.assertEqual(Hash256('DEFB4BF7ACF2145500087A02C88F8D1FCF27B8DEF4E0FDABE09413D87A3F0D09'), merkle_hash)

	def test_changing_sub_hash_order_changes_merkle_hash(self):
		# Arrange:
		seed_hashes1 = [TestUtils.random_byte_array(Hash256) for _ in range(0, 8)]
		seed_hashes2 = [seed_hashes1[i] for i in [0, 1, 2, 5, 4, 3, 6, 7]]

		# Sanity:
		self.assertEqual(len(seed_hashes1), len(seed_hashes2))

		# Act:
		merkle_hash1 = self._calculate_merkle_hash(seed_hashes1)
		merkle_hash2 = self._calculate_merkle_hash(seed_hashes2)

		# Assert:
		self.assertNotEqual(merkle_hash1, merkle_hash2)

	def test_changing_sub_hash_changes_merkle_hash(self):
		# Arrange:
		seed_hashes1 = [TestUtils.random_byte_array(Hash256) for _ in range(0, 8)]
		seed_hashes2 = [seed_hashes1[i] if 0 <= i else TestUtils.random_byte_array(Hash256) for i in [0, 1, 2, 3, -1, 5, 6, 7]]

		# Sanity:
		self.assertEqual(len(seed_hashes1), len(seed_hashes2))

		# Act:
		merkle_hash1 = self._calculate_merkle_hash(seed_hashes1)
		merkle_hash2 = self._calculate_merkle_hash(seed_hashes2)

		# Assert:
		self.assertNotEqual(merkle_hash1, merkle_hash2)

	@staticmethod
	def _calculate_merkle_hash(seed_hashes):
		builder = MerkleHashBuilder()

		for seed_hash in seed_hashes:
			builder.update(seed_hash)

		return builder.final()

# endregion


# region ProveMerkleTest

class ProveMerkleTest(unittest.TestCase):
	def test_succeeds_when_leaf_is_root_and_there_is_no_path(self):
		# Arrange:
		root_hash = Hash256('36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8')

		# Act:
		result = prove_merkle(root_hash, [], root_hash)

		# Assert:
		self.assertTrue(result)

	def test_fails_when_leaf_is_root_and_there_is_path(self):
		# Arrange:
		root_hash = Hash256('36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8')
		merkle_path = [
			MerklePart(Hash256('6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7'), True)
		]

		# Act:
		result = prove_merkle(root_hash, merkle_path, root_hash)

		# Assert:
		self.assertFalse(result)

	@staticmethod
	def _create_default_test_vector():
		return {
			'leaf_hash': Hash256('D4713ABB76AC98FB74AB91607C9029A95821C28462DC43707D92DD35E10B96CD'),
			'merkle_path': [
				MerklePart(Hash256('2CFB84D7A2F53FFAE07B1A686D84CB2491AD234F785B9C5905F1FF04E921F3F7'), False),
				MerklePart(Hash256('B49544CFA100301340F7F060C935B02687041431BC660E288176B1954D5DF5D0'), False),
				MerklePart(Hash256('0C346E96C61C4E54BCC10F1A4604C30C4A6D1E51691385BFFF2B9E56B2E0A9EB'), False),
				MerklePart(Hash256('399887ED3F5C3086A1DFF78B78697C1592E2C35C10FB45B5AAF621AB52D23B78'), True),
				MerklePart(Hash256('55DFB13E3F549DA89E9C38C97E7D2A557EE9B660EDA10DBD2088FB4CAFEF2524'), False),
				MerklePart(Hash256('190C27BF7B21C474E99E3FE0F3DBF437F33C784D80732C2F8263D3F6A0167C58'), False),
				MerklePart(Hash256('0B7DC05FA282E3BB156EE2861DF7E456AF538D56B4452996C39B4F5E46E2233E'), False),
				MerklePart(Hash256('60E4C788A881D84AFEA758C660E9C779A460F022AE3EC38D584155F08E84D82E'), True)
			],
			'root_hash': Hash256('DDDBA0604EE6A2CB9825CA5E0D31785F05F43713C5E7C512A900A7287DB5143C')
		}

	def test_succeeds_when_proof_is_valid(self):
		# Arrange:
		test_vector = self._create_default_test_vector()

		# Act:
		result = prove_merkle(test_vector['leaf_hash'], test_vector['merkle_path'], test_vector['root_hash'])

		# Assert:
		self.assertTrue(result)

	def test_fails_when_root_does_not_match(self):
		# Arrange:
		test_vector = self._create_default_test_vector()
		test_vector['root_hash'] = Hash256(test_vector['root_hash'].bytes[:-1] + bytes([test_vector['root_hash'].bytes[-1] ^ 0xFF]))

		# Act:
		result = prove_merkle(test_vector['leaf_hash'], test_vector['merkle_path'], test_vector['root_hash'])

		# Assert:
		self.assertFalse(result)

	def test_fails_when_branch_position_is_wrong(self):
		# Arrange:
		test_vector = self._create_default_test_vector()
		test_vector['merkle_path'][4] = MerklePart(test_vector['merkle_path'][4].hash, not test_vector['merkle_path'][4].is_left)

		# Act:
		result = prove_merkle(test_vector['leaf_hash'], test_vector['merkle_path'], test_vector['root_hash'])

		# Assert:
		self.assertFalse(result)

# endregion
