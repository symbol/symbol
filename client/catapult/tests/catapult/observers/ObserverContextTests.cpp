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

		void AssertContext(const ObserverContext& context, const cache::CatapultCacheDelta& cacheDelta, NotifyMode mode) {
			// Assert:
			EXPECT_EQ(&cacheDelta, &context.Cache);
			EXPECT_EQ(mode, context.Mode);

			EXPECT_EQ(Default_Height, context.Height);

			// - resolvers are copied into context and wired up correctly
			auto resolvedMosaicId = context.Resolvers.resolve(UnresolvedMosaicId(24));
			EXPECT_EQ(MosaicId(48), resolvedMosaicId);

			auto resolvedAddress = context.Resolvers.resolve(UnresolvedAddress{ { 11, 32 } });
			EXPECT_EQ(Address{ { 11 } }, resolvedAddress);
		}
	}

	TEST(TEST_CLASS, CanCreateCommitObserverContextWithoutBlockStatementBuilder) {
		// Act:
		RunTestWithCache([](auto& cacheDelta) {
			ObserverContext context(CreateNotificationContext(), ObserverState(cacheDelta), NotifyMode::Commit);
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, NotifyMode::Commit);
		});
	}

	TEST(TEST_CLASS, CanCreateCommitObserverContextWithBlockStatementBuilder) {
		// Act:
		RunTestWithCache([](auto& cacheDelta) {
			model::BlockStatementBuilder blockStatementBuilder;
			ObserverContext context(CreateNotificationContext(), ObserverState(cacheDelta, blockStatementBuilder), NotifyMode::Commit);
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, NotifyMode::Commit);

			auto pStatement = blockStatementBuilder.build();
			EXPECT_EQ(1u, pStatement->TransactionStatements.size()); // AddRandomReceipt
			EXPECT_EQ(1u, pStatement->AddressResolutionStatements.size()); // AssertContext
			EXPECT_EQ(1u, pStatement->MosaicResolutionStatements.size()); // AssertContext
		});
	}

	TEST(TEST_CLASS, CanCreateRollbackObserverContextWithoutBlockStatementBuilder) {
		// Act:
		RunTestWithCache([](auto& cacheDelta) {
			ObserverContext context(CreateNotificationContext(), ObserverState(cacheDelta), NotifyMode::Rollback);
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, NotifyMode::Rollback);
		});
	}

	TEST(TEST_CLASS, CanCreateRollbackObserverContextWithBlockStatementBuilder) {
		// Act: (this test is added for completeness because receipts are never generated during rollback)
		RunTestWithCache([](auto& cacheDelta) {
			model::BlockStatementBuilder blockStatementBuilder;
			ObserverContext context(CreateNotificationContext(), ObserverState(cacheDelta, blockStatementBuilder), NotifyMode::Rollback);
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, NotifyMode::Rollback);

			auto pStatement = blockStatementBuilder.build();
			EXPECT_EQ(1u, pStatement->TransactionStatements.size()); // AddRandomReceipt
			EXPECT_EQ(1u, pStatement->AddressResolutionStatements.size()); // AssertContext
			EXPECT_EQ(1u, pStatement->MosaicResolutionStatements.size()); // AssertContext
		});
	}

	// endregion
}}
