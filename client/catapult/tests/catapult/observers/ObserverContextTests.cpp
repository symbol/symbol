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
		void RunTestWithStateAndCache(TAction action) {
			// Arrange:
			cache::CatapultCache cache({});
			auto cacheDelta = cache.createDelta();
			state::CatapultState state;

			// Act:
			action(cacheDelta, state);
		}
	}

	// region ObserverState

	TEST(TEST_CLASS, CanCreateObserverState) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			ObserverState observerState(cacheDelta, state);

			// Assert:
			EXPECT_EQ(&cacheDelta, &observerState.Cache);
			EXPECT_EQ(&state, &observerState.State);
			EXPECT_FALSE(!!observerState.pBlockStatementBuilder);
		});
	}

	TEST(TEST_CLASS, CanCreateObserverStateWithBlockStatementBuilder) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			model::BlockStatementBuilder blockStatementBuilder;
			ObserverState observerState(cacheDelta, state, blockStatementBuilder);

			// Assert:
			EXPECT_EQ(&cacheDelta, &observerState.Cache);
			EXPECT_EQ(&state, &observerState.State);
			EXPECT_EQ(&blockStatementBuilder, observerState.pBlockStatementBuilder);
		});
	}

	// endregion

	// region ObserverContext

	namespace {
		constexpr auto Default_Height = Height(123);

		model::ResolverContext CreateResolverContext() {
			return model::ResolverContext(
					[](const auto& unresolved) { return MosaicId(unresolved.unwrap() * 2); },
					[](const auto& unresolved) { return Address{ { unresolved[0] } }; });
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
				const state::CatapultState& state,
				NotifyMode mode) {
			// Assert:
			EXPECT_EQ(&cacheDelta, &context.Cache);
			EXPECT_EQ(&state, &context.State);
			EXPECT_EQ(Default_Height, context.Height);
			EXPECT_EQ(mode, context.Mode);

			// - resolvers are copied into context and wired up correctly
			auto resolvedMosaicId = context.Resolvers.resolve(UnresolvedMosaicId(24));
			EXPECT_EQ(MosaicId(48), resolvedMosaicId);

			auto resolvedAddress = context.Resolvers.resolve(UnresolvedAddress{ { 11, 32 } });
			EXPECT_EQ(Address{ { 11 } }, resolvedAddress);
		}
	}

	TEST(TEST_CLASS, CanCreateCommitObserverContextWithoutBlockStatementBuilder) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			ObserverContext context(ObserverState(cacheDelta, state), Default_Height, NotifyMode::Commit, CreateResolverContext());
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, state, NotifyMode::Commit);
		});
	}

	TEST(TEST_CLASS, CanCreateCommitObserverContextWithBlockStatementBuilder) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			model::BlockStatementBuilder blockStatementBuilder;
			auto observerState = ObserverState(cacheDelta, state, blockStatementBuilder);
			ObserverContext context(observerState, Default_Height, NotifyMode::Commit, CreateResolverContext());
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, state, NotifyMode::Commit);

			auto pStatement = blockStatementBuilder.build();
			EXPECT_EQ(1u, pStatement->TransactionStatements.size()); // AddRandomReceipt
			EXPECT_EQ(1u, pStatement->AddressResolutionStatements.size()); // AssertContext
			EXPECT_EQ(1u, pStatement->MosaicResolutionStatements.size()); // AssertContext
		});
	}

	TEST(TEST_CLASS, CanCreateRollbackObserverContextWithoutBlockStatementBuilder) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			ObserverContext context(ObserverState(cacheDelta, state), Default_Height, NotifyMode::Rollback, CreateResolverContext());
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, state, NotifyMode::Rollback);
		});
	}

	TEST(TEST_CLASS, CanCreateRollbackObserverContextWithBlockStatementBuilder) {
		// Act: (this test is added for completeness because receipts are never generated during rollback)
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			model::BlockStatementBuilder blockStatementBuilder;
			auto observerState = ObserverState(cacheDelta, state, blockStatementBuilder);
			ObserverContext context(observerState, Default_Height, NotifyMode::Rollback, CreateResolverContext());
			AddRandomReceipt(context.StatementBuilder());

			// Assert:
			AssertContext(context, cacheDelta, state, NotifyMode::Rollback);

			auto pStatement = blockStatementBuilder.build();
			EXPECT_EQ(1u, pStatement->TransactionStatements.size()); // AddRandomReceipt
			EXPECT_EQ(1u, pStatement->AddressResolutionStatements.size()); // AssertContext
			EXPECT_EQ(1u, pStatement->MosaicResolutionStatements.size()); // AssertContext
		});
	}

	// endregion
}}
