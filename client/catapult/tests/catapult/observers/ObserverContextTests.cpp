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

#include "catapult/observers/ObserverContext.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS ObserverContextTests

	namespace {
		template<typename TAction>
		void RunTestWithCache(TAction action) {
			// Arrange:
			cache::CatapultCache cache({});
			auto cacheDelta = cache.createDelta();

			// Act:
			action(cacheDelta);
		}
	}

	// region ObserverState

	TEST(TEST_CLASS, CanCreateObserverState) {
		// Act:
		RunTestWithCache([](auto& cacheDelta) {
			ObserverState observerState(cacheDelta);

			// Assert:
			EXPECT_EQ(&cacheDelta, &observerState.Cache);
			EXPECT_FALSE(!!observerState.pBlockStatementBuilder);
		});
	}

	TEST(TEST_CLASS, CanCreateObserverStateWithBlockStatementBuilder) {
		// Act:
		RunTestWithCache([](auto& cacheDelta) {
			model::BlockStatementBuilder blockStatementBuilder;
			ObserverState observerState(cacheDelta, blockStatementBuilder);

			// Assert:
			EXPECT_EQ(&cacheDelta, &observerState.Cache);
			EXPECT_EQ(&blockStatementBuilder, observerState.pBlockStatementBuilder);
		});
	}

	// endregion

	// region ObserverContext

	namespace {
		constexpr auto Default_Height = Height(123);

		enum class ResolversType { Decorated, Undecorated };

		model::NotificationContext CreateNotificationContext() {
			auto resolvers = model::ResolverContext(
					[](const auto& unresolved) { return MosaicId(unresolved.unwrap() * 2); },
					[](const auto& unresolved) { return Address{ { unresolved[0] } }; });

			return model::NotificationContext(Default_Height, resolvers);
		}

		void AddRandomReceipt(ObserverStatementBuilder& statementBuilder) {
			model::Receipt receipt;
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&receipt), sizeof(model::Receipt) });
			receipt.Size = sizeof(model::Receipt);
			statementBuilder.addReceipt(receipt);
		}

		void AssertContext(
				const ObserverContext& context,
				const cache::CatapultCacheDelta& cacheDelta,
				NotifyMode mode,
				ResolversType resolversType) {
			// Assert:
			EXPECT_EQ(&cacheDelta, &context.Cache);
			EXPECT_EQ(mode, context.Mode);

			EXPECT_EQ(Default_Height, context.Height);

			// - resolvers are copied into context and wired up correctly
			const auto& resolvers = ResolversType::Decorated == resolversType ? context.Resolvers : context.UndecoratedResolvers;
			auto resolvedMosaicId = resolvers.resolve(UnresolvedMosaicId(24));
			EXPECT_EQ(MosaicId(48), resolvedMosaicId);

			auto resolvedAddress = resolvers.resolve(UnresolvedAddress{ { 11, 32 } });
			EXPECT_EQ(Address{ { 11 } }, resolvedAddress);
		}
	}

#define NOTIFY_MODE_TEST(TEST_NAME) \
	template<NotifyMode Notify_Mode> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Commit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NotifyMode::Commit>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Rollback) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NotifyMode::Rollback>(); } \
	template<NotifyMode Notify_Mode> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	NOTIFY_MODE_TEST(CanCreateObserverContextWithoutBlockStatementBuilder) {
		// Act:
		RunTestWithCache([](auto& cacheDelta) {
			ObserverContext context(CreateNotificationContext(), ObserverState(cacheDelta), Notify_Mode);
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, Notify_Mode, ResolversType::Decorated);
		});
	}

	NOTIFY_MODE_TEST(CanCreateObserverContextWithBlockStatementBuilder) {
		// Act:
		RunTestWithCache([](auto& cacheDelta) {
			model::BlockStatementBuilder blockStatementBuilder;
			ObserverContext context(CreateNotificationContext(), ObserverState(cacheDelta, blockStatementBuilder), Notify_Mode);
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, Notify_Mode, ResolversType::Decorated);

			auto pStatement = blockStatementBuilder.build();
			EXPECT_EQ(1u, pStatement->TransactionStatements.size()); // AddRandomReceipt
			EXPECT_EQ(1u, pStatement->AddressResolutionStatements.size()); // AssertContext
			EXPECT_EQ(1u, pStatement->MosaicResolutionStatements.size()); // AssertContext
		});
	}

	NOTIFY_MODE_TEST(CanCreateObserverContextWithoutBlockStatementBuilder_UndecoratedResolvers) {
		// Act:
		RunTestWithCache([](auto& cacheDelta) {
			ObserverContext context(CreateNotificationContext(), ObserverState(cacheDelta), Notify_Mode);
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, Notify_Mode, ResolversType::Undecorated);
		});
	}

	NOTIFY_MODE_TEST(CanCreateObserverContextWithBlockStatementBuilder_UndecoratedResolvers) {
		// Act:
		RunTestWithCache([](auto& cacheDelta) {
			model::BlockStatementBuilder blockStatementBuilder;
			ObserverContext context(CreateNotificationContext(), ObserverState(cacheDelta, blockStatementBuilder), Notify_Mode);
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, Notify_Mode, ResolversType::Undecorated);

			// - transaction statements are still created
			// - no resolution statements are created by the undecorated resolvers
			auto pStatement = blockStatementBuilder.build();
			EXPECT_EQ(1u, pStatement->TransactionStatements.size()); // AddRandomReceipt
			EXPECT_EQ(0u, pStatement->AddressResolutionStatements.size()); // AssertContext
			EXPECT_EQ(0u, pStatement->MosaicResolutionStatements.size()); // AssertContext
		});
	}

	// endregion
}}
