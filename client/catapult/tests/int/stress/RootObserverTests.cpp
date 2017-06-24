#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/local/PluginUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

	namespace {
		using NotifyMode = observers::NotifyMode;
		constexpr auto Num_Important_Accounts = 5u;

		auto CreateCacheForImportanceCalculationTests(plugins::PluginManager& pluginManager) {
			auto cache = pluginManager.createCache();
			auto delta = cache.createDelta();
			for (uint8_t i = 1u; i <= Num_Important_Accounts; ++i) {
				auto pState = delta.sub<cache::AccountStateCache>().addAccount(Key{ { i } }, Height(1));
				pState->Balances.credit(Xem_Id, Amount(i * 1'000'000));
				pState->ImportanceInfo.set(Importance(i * i * i), model::ImportanceHeight(1));
				pState->ImportanceInfo.set(Importance(i * i), model::ImportanceHeight(123));
			}

			cache.commit(Height());
			return cache;
		}

		void RootObserverNotify(
				const plugins::PluginManager& pluginManager,
				cache::CatapultCache& cache,
				state::CatapultState& state,
				const model::VerifiableEntity& entity,
				Height height,
				NotifyMode mode) {
			// Arrange:
			auto pRootObserver = CreateEntityObserver(pluginManager);
			auto delta = cache.createDelta();

			// Act:
			pRootObserver->notify(model::WeakEntityInfo(entity, Hash256()), observers::ObserverContext(delta, state, height, mode));
			cache.commit(Height());
		}
	}

	TEST(RootObserverTests, ExecuteCalculatesImportancesCorrectly) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();
		auto cache = CreateCacheForImportanceCalculationTests(*pPluginManager);
		state::CatapultState state;
		auto pBlock = test::GenerateEmptyRandomBlock();

		// Act:
		RootObserverNotify(*pPluginManager, cache, state, *pBlock, Height(246), NotifyMode::Commit);

		// Assert: importance should have been calculated from balances
		auto accountStateCacheView = cache.sub<cache::AccountStateCache>().createView();
		for (uint8_t i = 1u; i <= Num_Important_Accounts; ++i) {
			const auto message = "importance for account " + std::to_string(i);
			auto pState = accountStateCacheView->findAccount(Key{ { i } });
			EXPECT_EQ(model::ImportanceHeight(246), pState->ImportanceInfo.height()) << message;
			EXPECT_EQ(Importance(i), pState->ImportanceInfo.current()) << message;
		}
	}

	TEST(RootObserverTests, UndoCalculatesImportancesCorrectly) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();
		auto cache = CreateCacheForImportanceCalculationTests(*pPluginManager);
		state::CatapultState state;
		auto pBlock = test::GenerateEmptyRandomBlock();
		RootObserverNotify(*pPluginManager, cache, state, *pBlock, Height(246), NotifyMode::Commit);

		// Act:
		RootObserverNotify(*pPluginManager, cache, state, *pBlock, Height(246), NotifyMode::Rollback);

		// Assert: importance should have been restored from historical importances
		auto accountStateCacheView = cache.sub<cache::AccountStateCache>().createView();
		for (uint8_t i = 1u; i <= Num_Important_Accounts; ++i) {
			const auto message = "importance for account " + std::to_string(i);
			auto pState = accountStateCacheView->findAccount(Key{ { i } });
			EXPECT_EQ(model::ImportanceHeight(123), pState->ImportanceInfo.height()) << message;
			EXPECT_EQ(Importance(i * i), pState->ImportanceInfo.current()) << message;
		}
	}

	TEST(RootObserverTests, UndoCalculatesImportancesCorrectlyWhenMultipleCalculationsNeedToBeRolledBack) {
		// Arrange:
		auto pPluginManager = test::CreateDefaultPluginManager();
		auto cache = CreateCacheForImportanceCalculationTests(*pPluginManager);
		state::CatapultState state;
		auto pBlock = test::GenerateEmptyRandomBlock();
		for (auto height = Height(246); height <= Height(369); height = height + Height(1))
			RootObserverNotify(*pPluginManager, cache, state, *pBlock, height, NotifyMode::Commit);

		// Act:
		for (auto height = Height(369); height >= Height(246); height = height - Height(1))
			RootObserverNotify(*pPluginManager, cache, state, *pBlock, height, NotifyMode::Rollback);

		// Assert: importance should have been restored from historical importances
		auto accountStateCacheView = cache.sub<cache::AccountStateCache>().createView();
		for (uint8_t i = 1u; i <= Num_Important_Accounts; ++i) {
			const auto message = "importance for account " + std::to_string(i);
			auto pState = accountStateCacheView->findAccount(Key{ { i } });
			EXPECT_EQ(model::ImportanceHeight(123), pState->ImportanceInfo.height()) << message;
			EXPECT_EQ(Importance(i * i), pState->ImportanceInfo.current()) << message;
		}
	}
}}
