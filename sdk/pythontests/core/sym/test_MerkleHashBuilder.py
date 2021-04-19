import unittest

from symbolchain.core.CryptoTypes import Hash256
from symbolchain.core.sym.MerkleHashBuilder import MerkleHashBuilder

from ...test.NemTestUtils import NemTestUtils


class MerkleHashBuilderTest(unittest.TestCase):
    def test_can_build_from_zero_hashes(self):
        # Act:
        merkle_hash = self._calculate_merkle_hash([])

        # Assert:
        self.assertEqual(Hash256.zero(), merkle_hash)

    def test_can_build_from_one_hash(self):
        # Arrange:
        seed_hash = NemTestUtils.randcryptotype(Hash256)

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
            Hash256('410330530D04A277A7C96C1E4F34184FDEB0FFDA63563EFD796C404D7A6E5A20'),
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
        seed_hashes1 = [NemTestUtils.randcryptotype(Hash256) for _ in range(0, 8)]
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
        seed_hashes1 = [NemTestUtils.randcryptotype(Hash256) for _ in range(0, 8)]
        seed_hashes2 = [seed_hashes1[i] if 0 <= i else NemTestUtils.randcryptotype(Hash256) for i in [0, 1, 2, 3, -1, 5, 6, 7]]

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
