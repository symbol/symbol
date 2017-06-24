#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/crypto/Hashes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

	namespace {
		using Hashes = std::vector<Hash256>;

		Hash256 CalculateMerkleHash(const Hashes& hashes) {
			// Arrange:
			MerkleHashBuilder builder;

			// Act:
			for (const auto& hash : hashes)
				builder.update(hash);

			Hash256 merkleHash;
			builder.final(merkleHash);
			return merkleHash;
		}

		void AssertMerkleHash(const Hashes& hashes, const Hash256& expectedHash) {
			// Act:
			auto merkleHash = CalculateMerkleHash(hashes);

			// Assert:
			EXPECT_EQ(test::ToHexString(expectedHash), test::ToHexString(merkleHash));
		}
	}

	// region zero + one

	TEST(MerkleHashBuilderTests, CanBuildMerkleHashFromZeroHashes) {
		// Assert:
		AssertMerkleHash({}, Hash256());
	}

	TEST(MerkleHashBuilderTests, CanBuildMerkleHashFromSingleHash) {
		// Arrange:
		auto seedHash = test::GenerateRandomData<Hash256_Size>();

		// Assert:
		AssertMerkleHash({ seedHash }, seedHash);
	}

	// endregion

	// region multiple

	namespace {
		Hashes GenerateRandomHashes(size_t numHashes) {
			Hashes hashes(numHashes);
			for (auto i = 0u; i < numHashes; ++i)
				hashes[i] = test::GenerateRandomData<Hash256_Size>();

			return hashes;
		}

		Hashes Reduce(const Hashes& originalHashes) {
			if (0 != originalHashes.size() % 2)
				CATAPULT_THROW_RUNTIME_ERROR_1("original hashes must have even size", originalHashes.size());

			Hashes reducedHashes(originalHashes.size() / 2);
			for (auto i = 0u; i < originalHashes.size(); i+= 2)
				Sha3_256({ originalHashes[i].data(), 2 * Hash256_Size }, reducedHashes[i / 2]);

			return reducedHashes;
		}
	}

	TEST(MerkleHashBuilderTests, CanBuildMerkleHashFromBalancedTree) {
		// Arrange:
		auto seedHashes = GenerateRandomHashes(8);
		auto expectedHash = Reduce(Reduce(Reduce(seedHashes)))[0];

		// Assert:
		AssertMerkleHash(seedHashes, expectedHash);
	}

	TEST(MerkleHashBuilderTests, CanBuildMerkleHashFromUnbalancedTree) {
		// Arrange:
		auto seedHashes = GenerateRandomHashes(5);
		auto seedHashes2 = Reduce({ seedHashes[0], seedHashes[1], seedHashes[2], seedHashes[3], seedHashes[4], seedHashes[4] });
		auto seedHashes3 = Reduce({ seedHashes2[0], seedHashes2[1], seedHashes2[2], seedHashes2[2] });
		auto expectedHash = Reduce(seedHashes3)[0];

		// Assert:
		AssertMerkleHash(seedHashes, expectedHash);
	}

	TEST(MerkleHashBuilderTests, CanBuildMerkleHashFromBalancedTree_Deterministic) {
		// Arrange:
		auto seedHashes = {
			test::ToArray<Hash256_Size>("36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8"),
			test::ToArray<Hash256_Size>("8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28"),
			test::ToArray<Hash256_Size>("6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7"),
			test::ToArray<Hash256_Size>("2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681"),
			test::ToArray<Hash256_Size>("421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE"),
			test::ToArray<Hash256_Size>("7A1711AF5C402CFEFF87F6DA4B9C738100A7AC3EDAD38D698DF36CA3FE883480"),
			test::ToArray<Hash256_Size>("1E6516B2CC617E919FAE0CF8472BEB2BFF598F19C7A7A7DC260BC6715382822C"),
			test::ToArray<Hash256_Size>("410330530D04A277A7C96C1E4F34184FDEB0FFDA63563EFD796C404D7A6E5A20")
		};

		// Assert:
		AssertMerkleHash(seedHashes, test::ToArray<Hash256_Size>("7D853079F5F9EE30BDAE49C4956AF20CDF989647AFE971C069AC263DA1FFDF7E"));
	}

	TEST(MerkleHashBuilderTests, CanBuildMerkleHashFromUnbalancedTree_Deterministic) {
		// Arrange:
		auto seedHashes = {
			test::ToArray<Hash256_Size>("36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8"),
			test::ToArray<Hash256_Size>("8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28"),
			test::ToArray<Hash256_Size>("6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7"),
			test::ToArray<Hash256_Size>("2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681"),
			test::ToArray<Hash256_Size>("421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE")
		};

		// Assert:
		AssertMerkleHash(seedHashes, test::ToArray<Hash256_Size>("DEFB4BF7ACF2145500087A02C88F8D1FCF27B8DEF4E0FDABE09413D87A3F0D09"));
	}

	// endregion

	// region changes

	namespace {
		void AssertSignificantChange(size_t numHashes, const std::function<void (Hashes&)>& modifier) {
			// Arrange:
			auto seedHashes1 = GenerateRandomHashes(numHashes);
			auto seedHashes2 = seedHashes1;
			modifier(seedHashes2);

			// Act:
			auto merkleHash1 = CalculateMerkleHash(seedHashes1);
			auto merkleHash2 = CalculateMerkleHash(seedHashes2);

			// Assert:
			EXPECT_NE(test::ToHexString(merkleHash1), test::ToHexString(merkleHash2));

		}
	}

	TEST(MerkleHashBuilderTests, ChangingSubHashOrderChangesMerkleHash) {
		// Assert:
		AssertSignificantChange(8, [](auto& hashes) { std::swap(hashes[3], hashes[5]); });
	}

	TEST(MerkleHashBuilderTests, ChangingSubHashChangesMerkleHash) {
		// Assert:
		AssertSignificantChange(8, [](auto& hashes) { hashes[3][0] ^= 0xFF; });
	}

	// endregion
}}
