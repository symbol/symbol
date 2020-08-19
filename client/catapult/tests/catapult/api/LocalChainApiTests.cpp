/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "catapult/api/LocalChainApi.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace api {

#define TEST_CLASS LocalChainApiTests

	namespace {
		std::unique_ptr<ChainApi> CreateLocalChainApi(const io::BlockStorageCache& storage) {
			return api::CreateLocalChainApi(storage, []() { return model::ChainScore(1); }, []() { return Height(1); });
		}
	}

	// region chainStatistics

	TEST(TEST_CLASS, CanRetrieveChainStatistics) {
		// Arrange:
		auto numSupplierCalls = std::make_pair<size_t, size_t>(0, 0);
		auto chainScoreSupplier = [&numSupplierCalls]() {
			++numSupplierCalls.first;
			return model::ChainScore(12345);
		};
		auto finalizedHeightSupplier = [&numSupplierCalls]() {
			++numSupplierCalls.second;
			return Height(7);
		};

		auto pStorage = mocks::CreateMemoryBlockStorageCache(12);
		auto pApi = api::CreateLocalChainApi(*pStorage, chainScoreSupplier, finalizedHeightSupplier);

		// Act:
		auto chainStatistics = pApi->chainStatistics().get();

		// Assert:
		EXPECT_EQ(1u, numSupplierCalls.first);
		EXPECT_EQ(1u, numSupplierCalls.second);

		EXPECT_EQ(Height(12), chainStatistics.Height);
		EXPECT_EQ(Height(7), chainStatistics.FinalizedHeight);
		EXPECT_EQ(model::ChainScore(12345), chainStatistics.Score);
	}

	// endregion

	// region height-based errors

	namespace {
		struct HashesFromTraits {
			static auto Invoke(ChainApi& api, Height height) {
				// tests only check height, so number of hashes is not important
				return api.hashesFrom(height, 1);
			}
		};

		template<typename TTraits>
		void AssertApiErrorForHeight(uint32_t numBlocks, Height requestHeight) {
			// Arrange:
			auto pStorage = mocks::CreateMemoryBlockStorageCache(numBlocks);
			auto pApi = CreateLocalChainApi(*pStorage);

			// Act + Assert:
			auto future = TTraits::Invoke(*pApi, requestHeight);
			EXPECT_THROW(future.get(), catapult_api_error);
		}
	}

#define CHAIN_API_HEIGHT_ERROR_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_HashesFrom) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<HashesFromTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CHAIN_API_HEIGHT_ERROR_TRAITS_BASED_TEST(RequestAtHeightZeroFails) {
		AssertApiErrorForHeight<TTraits>(12, Height(0));
	}

	CHAIN_API_HEIGHT_ERROR_TRAITS_BASED_TEST(RequestAtHeightGreaterThanLocalChainHeightFails) {
		AssertApiErrorForHeight<TTraits>(12, Height(13));
		AssertApiErrorForHeight<TTraits>(12, Height(100));
	}

	// endregion

	// region hashesFrom

	namespace {
		void AssertCanRetrieveHashes(
				uint32_t numBlocks,
				uint32_t maxHashes,
				Height requestHeight,
				const std::vector<Height>& expectedHeights) {
			// Arrange:
			auto pStorage = mocks::CreateMemoryBlockStorageCache(numBlocks);
			auto pApi = CreateLocalChainApi(*pStorage);

			// Act:
			auto hashes = pApi->hashesFrom(requestHeight, maxHashes).get();

			// Assert:
			ASSERT_EQ(expectedHeights.size(), hashes.size());

			auto i = 0u;
			auto storageView = pStorage->view();
			for (const auto& hash : hashes) {
				// - calculate the expected hash
				auto pBlock = storageView.loadBlock(expectedHeights[i]);
				auto expectedHash = CalculateHash(*pBlock);

				// - compare
				EXPECT_EQ(expectedHash, hash) << "comparing hashes at " << i << " from " << requestHeight;
				++i;
			}
		}
	}

	TEST(TEST_CLASS, CanRetrieveAtMostMaxHashes) {
		AssertCanRetrieveHashes(12, 5, Height(3), { Height(3), Height(4), Height(5), Height(6), Height(7) });
	}

	TEST(TEST_CLASS, RetreivedHashesAreBoundedByLastBlock) {
		AssertCanRetrieveHashes(12, 10, Height(10), { Height(10), Height(11), Height(12) });
	}

	TEST(TEST_CLASS, CanRetrieveLastBlockHash) {
		AssertCanRetrieveHashes(12, 5, Height(12), { Height(12) });
	}

	// endregion
}}
