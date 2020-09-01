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

#include "finalization/src/FinalizationBootstrapperService.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/tests/test/FinalizationBootstrapperServiceTestUtils.h"
#include "finalization/tests/test/mocks/MockProofStorage.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace finalization {

#define TEST_CLASS FinalizationBootstrapperServiceTests

	// region FinalizationServerHooks

	namespace {
		struct MessageRangeConsumerTraits {
			using ParamType = model::AnnotatedEntityRange<model::FinalizationMessage>;

			static auto Get(const FinalizationServerHooks& hooks) {
				return hooks.messageRangeConsumer();
			}

			static void Set(FinalizationServerHooks& hooks, const handlers::RangeHandler<model::FinalizationMessage>& consumer) {
				hooks.setMessageRangeConsumer(consumer);
			}
		};
	}

#define CONSUMER_HOOK_TEST_ENTRY(TEST_NAME, CONSUMER_FACTORY_NAME) \
	TEST(TEST_CLASS, Hooks_##TEST_NAME##_##CONSUMER_FACTORY_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CONSUMER_FACTORY_NAME##Traits>(); \
	}

#define CONSUMER_HOOK_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	CONSUMER_HOOK_TEST_ENTRY(TEST_NAME, MessageRangeConsumer) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CONSUMER_HOOK_TEST(CannotAccessWhenUnset) {
		// Arrange:
		FinalizationServerHooks hooks;

		// Act + Assert:
		EXPECT_THROW(TTraits::Get(hooks), catapult_invalid_argument);
	}

	CONSUMER_HOOK_TEST(CanSetOnce) {
		// Arrange:
		FinalizationServerHooks hooks;

		typename TTraits::ParamType seedParam;
		const auto* pSeedParam = &seedParam;
		std::vector<decltype(pSeedParam)> consumedParams;

		TTraits::Set(hooks, [&consumedParams](auto&& param) {
			consumedParams.push_back(&param);
		});

		// Act:
		auto factory = TTraits::Get(hooks);
		ASSERT_TRUE(!!factory);

		factory(std::move(seedParam));

		// Assert: the param created above should be passed (and moved) down
		ASSERT_EQ(1u, consumedParams.size());
		EXPECT_EQ(pSeedParam, consumedParams[0]);
	}

	CONSUMER_HOOK_TEST(CannotSetMultipleTimes) {
		// Arrange:
		FinalizationServerHooks hooks;
		TTraits::Set(hooks, [](auto&&) {});

		// Act + Assert:
		EXPECT_THROW(TTraits::Set(hooks, [](auto&&) {}), catapult_invalid_argument);
	}

	// endregion

	// region FinalizationBootstrapperService - test context

	namespace {
		using VoterType = test::FinalizationBootstrapperServiceTestUtils::VoterType;

		constexpr auto Num_Services = test::FinalizationBootstrapperServiceTestUtils::Num_Bootstrapper_Services;

		struct FinalizationBootstrapperServiceTraits {
			static constexpr auto Ots_Key_Dilution = 7u;

			static auto CreateRegistrar(std::unique_ptr<io::ProofStorage>&& pProofStorage) {
				auto config = FinalizationConfiguration::Uninitialized();
				config.Size = 3000;
				config.Threshold = 2000;
				config.MaxHashesPerPoint = 64;
				config.OtsKeyDilution = Ots_Key_Dilution;
				config.VotingSetGrouping = 500;
				return CreateFinalizationBootstrapperServiceRegistrar(config, std::move(pProofStorage));
			}

			static auto CreateRegistrar() {
				return CreateRegistrar(std::make_unique<mocks::MockProofStorage>());
			}

			static auto CreateRegistrar(FinalizationPoint point, Height height, const Hash256& hash) {
				return CreateRegistrar(std::make_unique<mocks::MockProofStorage>(point, height, hash));
			}
		};

		using TestContext = test::VoterSeededCacheDependentServiceLocatorTestContext<FinalizationBootstrapperServiceTraits>;

		void AssertAggregatorProperties(
				const chain::MultiRoundMessageAggregatorView& aggregator,
				FinalizationPoint minPoint,
				FinalizationPoint maxPoint,
				const model::HeightHashPair& previousEstimate,
				const model::HeightHashPair& currentEstimate) {
			EXPECT_EQ(minPoint, aggregator.minFinalizationPoint());
			EXPECT_EQ(maxPoint, aggregator.maxFinalizationPoint());
			EXPECT_EQ(previousEstimate, aggregator.findEstimate(aggregator.maxFinalizationPoint() - FinalizationPoint(1)));
			EXPECT_EQ(currentEstimate, aggregator.findEstimate(aggregator.maxFinalizationPoint()));
		}

		void AssertAggregatorProperties(
				const chain::MultiRoundMessageAggregatorView& aggregator,
				FinalizationPoint minPoint,
				FinalizationPoint maxPoint,
				const model::HeightHashPair& previousEstimate) {
			AssertAggregatorProperties(aggregator, minPoint, maxPoint, previousEstimate, previousEstimate);
		}

		void AssertAggregatorCounters(
				const TestContext& context,
				FinalizationPoint minPoint,
				FinalizationPoint maxPoint,
				Height previousEstimateHeight,
				Height currentEstimateHeight) {
			EXPECT_EQ(minPoint.unwrap(), context.counter("FIN MIN FP"));
			EXPECT_EQ(maxPoint.unwrap(), context.counter("FIN MAX FP"));
			EXPECT_EQ(previousEstimateHeight.unwrap(), context.counter("FIN PREV EST"));
			EXPECT_EQ(currentEstimateHeight.unwrap(), context.counter("FIN CUR EST"));
		}
	}

	// endregion

	// region FinalizationBootstrapperService - basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(FinalizationBootstrapper, Initial)

	TEST(TEST_CLASS, MultiRoundMessageAggregatorServiceIsRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		auto lastFinalizedHash = test::GenerateRandomByteArray<Hash256>();
		context.boot(FinalizationPoint(12), Height(100), lastFinalizedHash);

		// Assert:
		EXPECT_EQ(Num_Services, context.locator().numServices());
		EXPECT_EQ(4u, context.locator().counters().size());

		// - service
		const auto& aggregator = GetMultiRoundMessageAggregator(context.locator());
		EXPECT_EQ(0u, aggregator.view().size());
		AssertAggregatorProperties(aggregator.view(), FinalizationPoint(12), FinalizationPoint(12), { Height(100), lastFinalizedHash });

		// - counters
		AssertAggregatorCounters(context, FinalizationPoint(12), FinalizationPoint(12), Height(100), Height(100));
	}

	TEST(TEST_CLASS, FinalizationHooksServiceIsRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Services, context.locator().numServices());

		// - service (get does not throw)
		GetFinalizationServerHooks(context.locator());
	}

	TEST(TEST_CLASS, ProofStorageServiceIsRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Services, context.locator().numServices());

		// - service (get does not throw)
		GetProofStorageCache(context.locator());
	}

	// endregion

	// region FinalizationBootstrapperService - hooks

	TEST(TEST_CLASS, LocalFinalizedHeightSupplierHookIsRegistered) {
		// Arrange:
		TestContext context;
		context.boot(FinalizationPoint(11), Height(123), Hash256());

		// Act:
		auto height = context.testState().state().hooks().localFinalizedHeightSupplier()();

		// Assert:
		EXPECT_EQ(Height(123), height);
	}

	// endregion

	// region FinalizationBootstrapperService - multi round message aggregator

	namespace {
		constexpr auto Stage = model::FinalizationStage::Prevote;

		template<typename TAction>
		void RunMultiRoundMessageAggregatorServiceTest(TAction action) {
			// Arrange:
			TestContext context;
			mocks::SeedStorageWithFixedSizeBlocks(context.testState().state().storage(), 25);

			auto lastFinalizedHash = test::GenerateRandomByteArray<Hash256>();
			context.boot(FinalizationPoint(12), Height(20), lastFinalizedHash);

			auto& aggregator = GetMultiRoundMessageAggregator(context.locator());

			// Act + Assert:
			action(aggregator, context, lastFinalizedHash);
		}
	}

	TEST(TEST_CLASS, MultiRoundMessageAggregatorServiceCountersAreCorrectWhenProcessingSingleRound) {
		// Arrange:
		RunMultiRoundMessageAggregatorServiceTest([](auto& aggregator, const auto& context, const auto& lastFinalizedHash) {
			// Act:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			aggregator.modifier().add(context.createMessage(VoterType::Large1, { FinalizationPoint(12), Stage }, Height(22), hash));

			// Assert:
			AssertAggregatorCounters(context, FinalizationPoint(12), FinalizationPoint(12), Height(20), Height(20));

			// - check aggregator
			EXPECT_EQ(1u, aggregator.view().size());
			AssertAggregatorProperties(aggregator.view(), FinalizationPoint(12), FinalizationPoint(12), { Height(20), lastFinalizedHash });
		});
	}

	TEST(TEST_CLASS, MultiRoundMessageAggregatorServiceCountersAreCorrectWhenProcessingMultipleRounds) {
		// Arrange:
		RunMultiRoundMessageAggregatorServiceTest([](auto& aggregator, const auto& context, const auto& lastFinalizedHash) {
			aggregator.modifier().setMaxFinalizationPoint(FinalizationPoint(15));

			// Act:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			aggregator.modifier().add(context.createMessage(VoterType::Large1, { FinalizationPoint(12), Stage }, Height(22), hash));
			aggregator.modifier().add(context.createMessage(VoterType::Large1, { FinalizationPoint(15), Stage }, Height(24), hash));

			// Assert:
			AssertAggregatorCounters(context, FinalizationPoint(12), FinalizationPoint(15), Height(20), Height(20));

			// - check aggregator
			EXPECT_EQ(2u, aggregator.view().size());
			AssertAggregatorProperties(aggregator.view(), FinalizationPoint(12), FinalizationPoint(15), { Height(20), lastFinalizedHash });
		});
	}

	TEST(TEST_CLASS, MultiRoundMessageAggregatorServiceCountersAreCorrectWhenProcessingMultipleRoundsWithNewEstimates) {
		// Arrange:
		RunMultiRoundMessageAggregatorServiceTest([](auto& aggregator, const auto& context, const auto&) {
			aggregator.modifier().setMaxFinalizationPoint(FinalizationPoint(15));

			// Act:
			auto hash1 = test::GenerateRandomByteArray<Hash256>();
			aggregator.modifier().add(context.createMessage(VoterType::Large1, { FinalizationPoint(12), Stage }, Height(22), hash1));
			aggregator.modifier().add(context.createMessage(VoterType::Large2, { FinalizationPoint(12), Stage }, Height(22), hash1));

			auto hash2 = test::GenerateRandomByteArray<Hash256>();
			aggregator.modifier().add(context.createMessage(VoterType::Large1, { FinalizationPoint(15), Stage }, Height(24), hash2));
			aggregator.modifier().add(context.createMessage(VoterType::Large2, { FinalizationPoint(15), Stage }, Height(24), hash2));

			// Assert:
			AssertAggregatorCounters(context, FinalizationPoint(12), FinalizationPoint(15), Height(22), Height(24));

			// - check aggregator
			EXPECT_EQ(2u, aggregator.view().size());
			AssertAggregatorProperties(
					aggregator.view(),
					FinalizationPoint(12),
					FinalizationPoint(15),
					{ Height(22), hash1 },
					{ Height(24), hash2 });
		});
	}

	// endregion
}}
