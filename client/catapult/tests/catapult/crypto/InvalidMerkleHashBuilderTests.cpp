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

#include "catapult/crypto/InvalidMerkleHashBuilder.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS InvalidMerkleHashBuilderTests

	namespace {
		using Hashes = std::vector<Hash256>;

		template<typename TTraits>
		auto CalculateMerkleResult(const Hashes& hashes) {
			// Arrange:
			InvalidMerkleHashBuilder builder;

			// Act:
			for (const auto& hash : hashes)
				builder.update(hash);

			typename TTraits::ResultType result;
			builder.final(result);
			return result;
		}

		auto HashTwo(const Hash256& hash1, const Hash256& hash2) {
			Hash256 result;

			crypto::Sha3_256_Builder builder;
			builder.update(hash1);
			builder.update(hash2);
			builder.final(result);

			return result;
		}

		auto CreateMerkleHashFromEightHashes(const Hashes& seedHashes) {
			// notice that only 4/8 seedHashes are protected from modification by the merkle hash
			auto seedHashes2 = Hashes(4);
			seedHashes2[0] = HashTwo(seedHashes[0], seedHashes[1]);
			seedHashes2[1] = HashTwo(seedHashes[1], seedHashes[2]);
			seedHashes2[2] = HashTwo(seedHashes[2], seedHashes[3]);
			seedHashes2[3] = HashTwo(seedHashes[3], seedHashes[4]);

			auto seedHashes3 = Hashes(2);
			seedHashes3[0] = HashTwo(seedHashes2[0], seedHashes2[1]); // derived from [ 0, 1, 2 ]
			seedHashes3[1] = HashTwo(seedHashes2[1], seedHashes2[2]); // derived from    [ 1, 2, 3 ]
			return HashTwo(seedHashes3[0], seedHashes3[1]);
		}

		auto CreateMerkleHashFromFiveHashes(const Hashes& seedHashes) {
			// notice that only 3/5 seedHashes are protected from modification by the merkle hash
			auto seedHashes2 = Hashes(3);
			seedHashes2[0] = HashTwo(seedHashes[0], seedHashes[1]);
			seedHashes2[1] = HashTwo(seedHashes[1], seedHashes[2]);
			seedHashes2[2] = HashTwo(seedHashes[2], seedHashes[2]);

			auto seedHashes3 = Hashes(2);
			seedHashes3[0] = HashTwo(seedHashes2[0], seedHashes2[1]); // derived from [ 0, 1, 2 ]
			seedHashes3[1] = HashTwo(seedHashes2[1], seedHashes2[1]); // derived from    [ 1, 2 ]
			return HashTwo(seedHashes3[0], seedHashes3[1]);
		}

		auto CreateMerkleHash(const Hashes& seedHashes) {
			switch (seedHashes.size()) {
			case 0:
				return Hash256();
			case 1:
				return seedHashes[0];
			case 5:
				return CreateMerkleHashFromFiveHashes(seedHashes);
			case 8:
				return CreateMerkleHashFromEightHashes(seedHashes);
			default:
				CATAPULT_THROW_RUNTIME_ERROR_1("creating merkle tree not supported for this number of hashes", seedHashes.size());
			}
		}

		struct MerkleHashTraits {
			using ResultType = Hash256;

			static ResultType PrepareExpected(const Hashes& hashes) {
				return CreateMerkleHash(hashes);
			}

			static const ResultType DeterministicResultBalanced() {
				return utils::ParseByteArray<Hash256>("59D6AB7B7809AE1D5953860E756A96425EAD8EF4B9A30EB9AE1283D68CC0BEB5");
			}

			static const ResultType DeterministicResultUnbalanced() {
				return utils::ParseByteArray<Hash256>("12E6F9716BB13BBD73081296DEA62388C178A2B9119C7D2F40B35A34F30A09FF");
			}

			static void AssertResult(const Hashes& hashes, const ResultType& expectedResult) {
				// Act:
				auto result = CalculateMerkleResult<MerkleHashTraits>(hashes);

				// Assert:
				EXPECT_EQ(expectedResult, result);
			}
		};
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_MerkleHash) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MerkleHashTraits>(); } \
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
}}
