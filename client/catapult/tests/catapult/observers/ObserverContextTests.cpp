#include "catapult/observers/ObserverContext.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS ObserverContextTests

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

	TEST(TEST_CLASS, CanCreateObserverState) {
		// Act:
		RunTestWithStateAndCache([](auto& cacheDelta, auto& state) {
			ObserverState observerState(cacheDelta, state);

			// Assert:
			EXPECT_EQ(&cacheDelta, &observerState.Cache);
			EXPECT_EQ(&state, &observerState.State);
		});
	}

	TEST(TEST_CLASS, CanCreateCommitObserverContextAroundObserverState) {
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

	TEST(TEST_CLASS, CanCreateCommitObserverContextAroundCacheAndState) {
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

	TEST(TEST_CLASS, CanCreateRollbackObserverContextAroundObserverState) {
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

	TEST(TEST_CLASS, CanCreateRollbackObserverContextAroundCacheAndState) {
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
