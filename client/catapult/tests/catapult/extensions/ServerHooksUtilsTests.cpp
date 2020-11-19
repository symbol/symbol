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

#include "catapult/extensions/ServerHooksUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ServerHooksUtilsTests

	// region CreateXyzPushEntityCallback

	namespace {
		struct BlockTraits {
			static constexpr auto CreateRange = test::CreateBlockEntityRange;
			static constexpr auto CreatePushEntityCallback = CreateBlockPushEntityCallback;

			template<typename TConsumerFactory>
			static void SetConsumerFactory(ServerHooks& hooks, TConsumerFactory consumerFactory) {
				hooks.setBlockRangeConsumerFactory(consumerFactory);
			}
		};

		struct TransactionTraits {
			static constexpr auto CreateRange = test::CreateTransactionEntityRange;
			static constexpr auto CreatePushEntityCallback = CreateTransactionPushEntityCallback;

			template<typename TConsumerFactory>
			static void SetConsumerFactory(ServerHooks& hooks, TConsumerFactory consumerFactory) {
				hooks.setTransactionRangeConsumerFactory(consumerFactory);
			}
		};

		template<typename TTraits>
		void AssertPush(bool isChainSynced) {
			// Arrange:
			std::vector<disruptor::InputSource> sources;
			std::vector<decltype(TTraits::CreateRange(0))> ranges;

			ServerHooks hooks;
			hooks.setChainSyncedPredicate([isChainSynced]() { return isChainSynced; });
			TTraits::SetConsumerFactory(hooks, [&sources, &ranges](auto source) {
				sources.push_back(source);
				return [&ranges](auto&& range) {
					ranges.push_back(std::move(range.Range));
				};
			});

			auto range = TTraits::CreateRange(3);
			const auto* pRangeData = range.data();

			// Act:
			TTraits::CreatePushEntityCallback(hooks)(std::move(range));

			// Assert: consumer should always be created exactly once
			ASSERT_EQ(1u, sources.size());
			EXPECT_EQ(disruptor::InputSource::Remote_Push, sources[0]);

			// - range data is only pushed when the chain is synced
			ASSERT_EQ(isChainSynced ? 1u : 0u, ranges.size());
			if (isChainSynced)
				EXPECT_EQ(pRangeData, ranges[0].data());
		}
	}

#define PUSH_ENTITY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Transaction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	PUSH_ENTITY_TEST(CanPushWhenSynced) {
		AssertPush<TTraits>(true);
	}

	PUSH_ENTITY_TEST(CannotPushWhenNotSynced) {
		AssertPush<TTraits>(false);
	}

	// endregion
}}
