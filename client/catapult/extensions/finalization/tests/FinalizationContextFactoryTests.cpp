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

#include "finalization/src/FinalizationContextFactory.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace finalization {

#define TEST_CLASS FinalizationContextFactoryTests

	namespace {
		void AssertCanCreateFinalizationContext(Height height, Height groupedHeight) {
			// Arrange: setup config
			auto config = finalization::FinalizationConfiguration::Uninitialized();
			config.Size = 9876;
			config.VotingSetGrouping = 50;

			auto blockChainConfig = test::CreatePrototypicalBlockChainConfiguration();
			blockChainConfig.MinVoterBalance = Amount(2'000'000);
			blockChainConfig.VotingSetGrouping = config.VotingSetGrouping;

			// - create a cache and seed accounts at groupedHeight (15M) and height (11M)
			auto catapultCache = test::CoreSystemCacheFactory::Create(blockChainConfig);
			{
				auto catapultCacheDelta = catapultCache.createDelta();
				auto& accountStateCacheDelta = catapultCacheDelta.sub<cache::AccountStateCache>();

				test::AddAccountsWithBalances(accountStateCacheDelta, groupedHeight, blockChainConfig.HarvestingMosaicId, {
					Amount(4'000'000), Amount(2'000'000), Amount(1'000'000), Amount(2'000'000), Amount(3'000'000), Amount(4'000'000)
				});
				test::AddAccountsWithBalances(accountStateCacheDelta, height, blockChainConfig.HarvestingMosaicId, {
					Amount(4'000'000), Amount(2'000'000), Amount(1'000'000), Amount(2'000'000), Amount(3'000'000)
				});

				catapultCache.commit(height);
			}

			test::ServiceTestState testState(std::move(catapultCache));
			mocks::SeedStorageWithFixedSizeBlocks(testState.state().storage(), static_cast<uint32_t>(groupedHeight.unwrap()));

			// Act:
			FinalizationContextFactory factory(config, testState.state());
			auto context = factory.create(FinalizationPoint(12), height);

			// Assert: context should be seeded with data from groupedHeight, not height
			auto expectedGenerationHash = testState.state().storage().view().loadBlockElement(groupedHeight)->GenerationHash;

			EXPECT_EQ(FinalizationPoint(12), context.point());
			EXPECT_EQ(groupedHeight, context.height());
			EXPECT_EQ(expectedGenerationHash, context.generationHash());
			EXPECT_EQ(9876u, context.config().Size);
			EXPECT_EQ(Amount(15'000'000), context.weight());
		}
	}

	TEST(TEST_CLASS, CanCreateFinalizationContextAtVotingSetGroupStart) {
		AssertCanCreateFinalizationContext(Height(51), Height(50));
	}

	TEST(TEST_CLASS, CanCreateFinalizationContextAtVotingSetGroupMiddle) {
		AssertCanCreateFinalizationContext(Height(76), Height(50));
	}

	TEST(TEST_CLASS, CanCreateFinalizationContextAtVotingSetGroupEnd) {
		AssertCanCreateFinalizationContext(Height(100), Height(50));
	}
}}
