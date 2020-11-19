/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS MerkleHashBuilderTests

	namespace {
		using Hashes = std::vector<Hash256>;

		template<typename TTraits>
		auto CalculateMerkleResult(const Hashes& hashes) {
			// Arrange:
			MerkleHashBuilder builder;

			// Act:
			for (const auto& hash : hashes)
				builder.update(hash);

			typename TTraits::ResultType result;
			builder.final(result);
			return result;
		}

		Hashes Reduce(const Hashes& originalHashes) {
			if (0 != originalHashes.size() % 2)
				CATAPULT_THROW_RUNTIME_ERROR_1("original hashes must have even size", originalHashes.size());

			Hashes reducedHashes(originalHashes.size() / 2);
			for (auto i = 0u; i < originalHashes.size(); i += 2)
				Sha3_256({ originalHashes[i].data(), 2 * Hash256::Size }, reducedHashes[i / 2]);

			return reducedHashes;
		}

		auto CreateMerkleTreeFromEightHashes(const Hashes& seedHashes) {
			Hashes tree(seedHashes.cbegin(), seedHashes.cend());
			auto hashes = seedHashes;
			for (auto i = 0u; i < 3; ++i) {
				hashes = Reduce(hashes);
				tree.insert(tree.cend(), hashes.cbegin(), hashes.cend());
			}

			return tree;
		}

		auto CreateMerkleTreeFromFiveHashes(const Hashes& seedHashes) {
			Hashes tree(seedHashes.cbegin(), seedHashes.cend());
			tree.push_back(*--seedHashes.cend());
			auto seedHashes2 = Reduce({ seedHashes[0], seedHashes[1], seedHashes[2], seedHashes[3], seedHashes[4], seedHashes[4] });
			tree.insert(tree.cend(), seedHashes2.cbegin(), seedHashes2.cend());
			tree.push_back(*--seedHashes2.cend());
			auto seedHashes3 = Reduce({ seedHashes2[0], seedHashes2[1], seedHashes2[2], seedHashes2[2] });
			tree.insert(tree.cend(), seedHashes3.cbegin(), seedHashes3.cend());
			tree.push_back(Reduce(seedHashes3)[0]);
			return tree;
		}

		auto CreateMerkleTree(const Hashes& seedHashes) {
			switch (seedHashes.size()) {
			case 0:
				return Hashes{ Hash256() };
			case 1:
				return Hashes{ seedHashes[0] };
			case 5:
				return CreateMerkleTreeFromFiveHashes(seedHashes);
			case 8:
				return CreateMerkleTreeFromEightHashes(seedHashes);
			default:
				CATAPULT_THROW_RUNTIME_ERROR_1("creating merkle tree not supported for this number of hashes", seedHashes.size());
			}
		}

		struct MerkleHashTraits {
			using ResultType = Hash256;

			static ResultType PrepareExpected(const Hashes& hashes) {
				auto tree = CreateMerkleTree(hashes);
				return *--tree.cend();
			}

			static const ResultType DeterministicResultBalanced() {
				return utils::ParseByteArray<Hash256>("7D853079F5F9EE30BDAE49C4956AF20CDF989647AFE971C069AC263DA1FFDF7E");
			}

			static const ResultType DeterministicResultUnbalanced() {
				return utils::ParseByteArray<Hash256>("DEFB4BF7ACF2145500087A02C88F8D1FCF27B8DEF4E0FDABE09413D87A3F0D09");
			}

			static void AssertResult(const Hashes& hashes, const ResultType& expectedResult) {
				// Act:
				auto result = CalculateMerkleResult<MerkleHashTraits>(hashes);

				// Assert:
				EXPECT_EQ(expectedResult, result);
			}
		};

		struct MerkleTreeTraits {
			using ResultType = Hashes;

			static ResultType PrepareExpected(const Hashes& hashes) {
				return CreateMerkleTree(hashes);
			}

			static const ResultType DeterministicResultBalanced() {
				return {
					utils::ParseByteArray<Hash256>("36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8"),
					utils::ParseByteArray<Hash256>("8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28"),
					utils::ParseByteArray<Hash256>("6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7"),
					utils::ParseByteArray<Hash256>("2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681"),
					utils::ParseByteArray<Hash256>("421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE"),
					utils::ParseByteArray<Hash256>("7A1711AF5C402CFEFF87F6DA4B9C738100A7AC3EDAD38D698DF36CA3FE883480"),
					utils::ParseByteArray<Hash256>("1E6516B2CC617E919FAE0CF8472BEB2BFF598F19C7A7A7DC260BC6715382822C"),
					utils::ParseByteArray<Hash256>("410330530D04A277A7C96C1E4F34184FDEB0FFDA63563EFD796C404D7A6E5A20"),
					utils::ParseByteArray<Hash256>("5C3BF2D64B151451DB3189EA569CC3BCB022E62D2C3C9FCC88B4683C6B550849"),
					utils::ParseByteArray<Hash256>("4B5B85209A686C423A278EE0B553375C247C479221DB041392D2876CBE167A43"),
					utils::ParseByteArray<Hash256>("135C564CAD34FF27C98CBC07FCBD13591596B9EB08B41B2C4B29FB137CBFBE84"),
					utils::ParseByteArray<Hash256>("62A067A76E7806772E2EA59AD61B8E3F15156698B7A1A6457709204D5E2EF356"),
					utils::ParseByteArray<Hash256>("3C8C43FB0425AB4E458363560703044BEF01A2F84D28B42A6BA60AFAA2F0A15D"),
					utils::ParseByteArray<Hash256>("35610FA7E92A198913975952443DD198A8FFADF0BB3D780BBE29E82EA0B1C22F"),
					utils::ParseByteArray<Hash256>("7D853079F5F9EE30BDAE49C4956AF20CDF989647AFE971C069AC263DA1FFDF7E")
				};
			}

			static const ResultType DeterministicResultUnbalanced() {
				return {
					utils::ParseByteArray<Hash256>("36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8"),
					utils::ParseByteArray<Hash256>("8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28"),
					utils::ParseByteArray<Hash256>("6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7"),
					utils::ParseByteArray<Hash256>("2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681"),
					utils::ParseByteArray<Hash256>("421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE"),
					utils::ParseByteArray<Hash256>("421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE"),
					utils::ParseByteArray<Hash256>("5C3BF2D64B151451DB3189EA569CC3BCB022E62D2C3C9FCC88B4683C6B550849"),
					utils::ParseByteArray<Hash256>("4B5B85209A686C423A278EE0B553375C247C479221DB041392D2876CBE167A43"),
					utils::ParseByteArray<Hash256>("5E0595BF035076842751B01D06C7F139FA0A190C9ACFB582A25B449A62522E34"),
					utils::ParseByteArray<Hash256>("5E0595BF035076842751B01D06C7F139FA0A190C9ACFB582A25B449A62522E34"),
					utils::ParseByteArray<Hash256>("3C8C43FB0425AB4E458363560703044BEF01A2F84D28B42A6BA60AFAA2F0A15D"),
					utils::ParseByteArray<Hash256>("D27767C5F49035071EBE402A495D0152E5B9307C86AF43BBE6FE3D35D41F245F"),
					utils::ParseByteArray<Hash256>("DEFB4BF7ACF2145500087A02C88F8D1FCF27B8DEF4E0FDABE09413D87A3F0D09")
				};
			}

			static void AssertResult(const Hashes& hashes, const ResultType& expectedResult) {
				// Act:
				auto result = CalculateMerkleResult<MerkleTreeTraits>(hashes);

				// Assert:
				for (auto i = 0u; i < result.size(); ++i)
					EXPECT_EQ(expectedResult[i], result[i]) << "at index " << i;
			}
		};
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_MerkleHash) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MerkleHashTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MerkleTree) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MerkleTreeTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region final - zero + one

	TRAITS_BASED_TEST(CanBuildResultFromZeroHashes) {
		TTraits::AssertResult({}, TTraits::PrepareExpected({}));
	}

	TRAITS_BASED_TEST(CanBuildResultFromSingleHash) {
		// Arrange:
		auto seedHash = test::GenerateRandomByteArray<Hash256>();

		// Assert:
		TTraits::AssertResult({ seedHash }, TTraits::PrepareExpected({ seedHash }));
	}

	// endregion

	// region final - multiple

	namespace {
		Hashes GenerateRandomHashes(size_t numHashes) {
			Hashes hashes(numHashes);
			for (auto i = 0u; i < numHashes; ++i)
				hashes[i] = test::GenerateRandomByteArray<Hash256>();

			return hashes;
		}
	}

	TRAITS_BASED_TEST(CanBuildResultFromBalancedTree) {
		// Arrange:
		auto seedHashes = GenerateRandomHashes(8);

		// Assert:
		TTraits::AssertResult(seedHashes, TTraits::PrepareExpected(seedHashes));
	}

	TRAITS_BASED_TEST(CanBuildResultFromUnbalancedTree) {
		// Arrange:
		auto seedHashes = GenerateRandomHashes(5);

		// Assert:
		TTraits::AssertResult(seedHashes, TTraits::PrepareExpected(seedHashes));
	}

	TRAITS_BASED_TEST(CanBuildResultFromBalancedTree_Deterministic) {
		// Arrange:
		auto seedHashes = {
			utils::ParseByteArray<Hash256>("36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8"),
			utils::ParseByteArray<Hash256>("8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28"),
			utils::ParseByteArray<Hash256>("6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7"),
			utils::ParseByteArray<Hash256>("2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681"),
			utils::ParseByteArray<Hash256>("421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE"),
			utils::ParseByteArray<Hash256>("7A1711AF5C402CFEFF87F6DA4B9C738100A7AC3EDAD38D698DF36CA3FE883480"),
			utils::ParseByteArray<Hash256>("1E6516B2CC617E919FAE0CF8472BEB2BFF598F19C7A7A7DC260BC6715382822C"),
			utils::ParseByteArray<Hash256>("410330530D04A277A7C96C1E4F34184FDEB0FFDA63563EFD796C404D7A6E5A20")
		};

		// Assert:
		TTraits::AssertResult(seedHashes, TTraits::DeterministicResultBalanced());
	}

	TRAITS_BASED_TEST(CanBuildResultFromUnbalancedTree_Deterministic) {
		// Arrange:
		auto seedHashes = {
			utils::ParseByteArray<Hash256>("36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8"),
			utils::ParseByteArray<Hash256>("8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28"),
			utils::ParseByteArray<Hash256>("6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7"),
			utils::ParseByteArray<Hash256>("2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681"),
			utils::ParseByteArray<Hash256>("421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE")
		};

		// Assert:
		TTraits::AssertResult(seedHashes, TTraits::DeterministicResultUnbalanced());
	}

	// endregion

	// region final - changes

	namespace {
		template<typename TTraits>
		void AssertSignificantChange(size_t numHashes, const consumer<Hashes&>& modifier) {
			// Arrange:
			auto seedHashes1 = GenerateRandomHashes(numHashes);
			auto seedHashes2 = seedHashes1;
			modifier(seedHashes2);

			// Act:
			auto result1 = CalculateMerkleResult<TTraits>(seedHashes1);
			auto result2 = CalculateMerkleResult<TTraits>(seedHashes2);

			// Assert:
			EXPECT_NE(result1, result2);

		}
	}

	TRAITS_BASED_TEST(ChangingSubHashOrderChangesMerkleHash) {
		AssertSignificantChange<TTraits>(8, [](auto& hashes) { std::swap(hashes[3], hashes[5]); });
	}

	TRAITS_BASED_TEST(ChangingSubHashChangesMerkleHash) {
		AssertSignificantChange<TTraits>(8, [](auto& hashes) { hashes[3][0] ^= 0xFF; });
	}

	// endregion

	// region treeSize

	TEST(TEST_CLASS, TreeSizeReturnsExpectedValue) {
		// Arrange:
		std::vector<std::pair<size_t, size_t>> expectedSizePairs{ { 0, 1 }, { 1, 1 }, { 2, 3 }, { 3, 7 }, { 4, 7 }, { 7, 15 }, { 8, 15 } };

		for (const auto& pair : expectedSizePairs) {
			// Act:
			auto treeSize = MerkleHashBuilder::TreeSize(pair.first);

			// Assert:
			EXPECT_EQ(pair.second, treeSize) << "for leaf count " << pair.first;
		}
	}

	// endregion
}}
