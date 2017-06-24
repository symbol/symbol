#include "catapult/observers/ObserverContext.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

	namespace {
		constexpr auto Default_Height = Height(123);

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

	TEST(ObserverContextTests, CanCreateObserverState) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			ObserverState observerState(cacheDelta, state);

			// Assert:
			EXPECT_EQ(&cacheDelta, &observerState.Cache);
			EXPECT_EQ(&state, &observerState.State);
		});
	}

	TEST(ObserverContextTests, CanCreateCommitObserverContextAroundObserverState) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			ObserverContext context(ObserverState(cacheDelta, state), Default_Height, NotifyMode::Commit);

			// Assert:
			EXPECT_EQ(&cacheDelta, &context.Cache);
			EXPECT_EQ(&state, &context.State);
			EXPECT_EQ(Default_Height, context.Height);
			EXPECT_EQ(NotifyMode::Commit, context.Mode);
		});
	}

	TEST(ObserverContextTests, CanCreateCommitObserverContextAroundCacheAndState) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			ObserverContext context(cacheDelta, state, Default_Height, NotifyMode::Commit);

			// Assert:
			EXPECT_EQ(&cacheDelta, &context.Cache);
			EXPECT_EQ(&state, &context.State);
			EXPECT_EQ(Default_Height, context.Height);
			EXPECT_EQ(NotifyMode::Commit, context.Mode);
		});
	}

	TEST(ObserverContextTests, CanCreateRollbackObserverContextAroundObserverState) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			ObserverContext context(ObserverState(cacheDelta, state), Default_Height, NotifyMode::Rollback);

			// Assert:
			EXPECT_EQ(&cacheDelta, &context.Cache);
			EXPECT_EQ(&state, &context.State);
			EXPECT_EQ(Default_Height, context.Height);
			EXPECT_EQ(NotifyMode::Rollback, context.Mode);
		});
	}

	TEST(ObserverContextTests, CanCreateRollbackObserverContextAroundCacheAndState) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			ObserverContext context(cacheDelta, state, Default_Height, NotifyMode::Rollback);

			// Assert:
			EXPECT_EQ(&cacheDelta, &context.Cache);
			EXPECT_EQ(&state, &context.State);
			EXPECT_EQ(Default_Height, context.Height);
			EXPECT_EQ(NotifyMode::Rollback, context.Mode);
		});
	}
}}
