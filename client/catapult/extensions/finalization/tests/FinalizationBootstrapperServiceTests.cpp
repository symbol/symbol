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

#include "finalization/src/FinalizationBootstrapperService.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/io/FilePrevoteChainStorage.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "finalization/src/model/FinalizationProofUtils.h"
#include "finalization/tests/test/FinalizationBootstrapperServiceTestUtils.h"
#include "finalization/tests/test/mocks/MockProofStorage.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace finalization {

#define TEST_CLASS FinalizationBootstrapperServiceTests
#define PHASE_TWO_TEST_CLASS FinalizationBootstrapperPhaseTwoServiceTests

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

		constexpr auto Finalization_Epoch = FinalizationEpoch(2);
		constexpr auto Num_Services = test::FinalizationBootstrapperServiceTestUtils::Num_Bootstrapper_Services;

		struct FinalizationBootstrapperServiceTraits {
			static auto CreateRegistrar(std::unique_ptr<io::ProofStorage>&& pProofStorage) {
				auto config = FinalizationConfiguration::Uninitialized();
				config.Size = 3000;
				config.Threshold = 2000;
				config.MaxHashesPerPoint = 64;
				config.VotingSetGrouping = 500;
				return CreateFinalizationBootstrapperServiceRegistrar(config, std::move(pProofStorage));
			}

			static auto CreateRegistrar() {
				return CreateRegistrar(std::make_unique<mocks::MockProofStorage>());
			}

			static auto CreateRegistrar(FinalizationPoint point, Height height, const Hash256& hash) {
				return CreateRegistrar(std::make_unique<mocks::MockProofStorage>(Finalization_Epoch, point, height, hash));
			}
		};

		using TestContext = test::VoterSeededCacheDependentServiceLocatorTestContext<FinalizationBootstrapperServiceTraits>;

		void AssertAggregatorProperties(
				const chain::MultiRoundMessageAggregatorView& aggregator,
				FinalizationPoint minPoint,
				FinalizationPoint maxPoint,
				const model::HeightHashPair& previousEstimate,
				const model::HeightHashPair& currentEstimate) {
			EXPECT_EQ(model::FinalizationRound({ Finalization_Epoch, minPoint }), aggregator.minFinalizationRound());
			EXPECT_EQ(model::FinalizationRound({ Finalization_Epoch, maxPoint }), aggregator.maxFinalizationRound());
			EXPECT_EQ(previousEstimate, aggregator.findEstimate(aggregator.maxFinalizationRound() - FinalizationPoint(1)));
			EXPECT_EQ(currentEstimate, aggregator.findEstimate(aggregator.maxFinalizationRound()));
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
			EXPECT_EQ(Finalization_Epoch.unwrap(), context.counter("FIN MIN EPOCH"));
			EXPECT_EQ(minPoint.unwrap(), context.counter("FIN MIN POINT"));
			EXPECT_EQ(Finalization_Epoch.unwrap(), context.counter("FIN MAX EPOCH"));
			EXPECT_EQ(maxPoint.unwrap(), context.counter("FIN MAX POINT"));
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
		EXPECT_EQ(6u, context.locator().counters().size());

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

	namespace {
		std::unique_ptr<model::FinalizationProof> CreateProof(uint32_t epoch, uint32_t point, Height height, const Hash256& hash) {
			auto pProof = std::make_unique<model::FinalizationProof>();
			pProof->Size = sizeof(model::FinalizationProof);
			pProof->Round = { FinalizationEpoch(epoch), FinalizationPoint(point), };
			pProof->Height = height;
			pProof->Hash = hash;
			return pProof;
		}

		void RunLocalFinalizedHeightHashPairSupplierHookTest(Height localProofHeight, uint32_t localChainHeight) {
			// Arrange:
			TestContext context;
			mocks::SeedStorageWithFixedSizeBlocks(context.testState().state().storage(), localChainHeight);

			auto proofHashes = test::GenerateRandomDataVector<Hash256>(4);
			auto loadHash = [&context, &proofHashes, localProofHeight](auto height, size_t index) {
				return localProofHeight == height
						? context.testState().state().storage().view().loadBlockElement(height)->EntityHash
						: proofHashes[index];
			};

			auto pProofStorage = std::make_unique<mocks::MockProofStorage>();
			pProofStorage->setLastFinalizationProof(CreateProof(1, 1, Height(1), loadHash(Height(1), 0)));
			pProofStorage->setLastFinalizationProof(CreateProof(2, 4, Height(10), loadHash(Height(10), 1)));
			pProofStorage->setLastFinalizationProof(CreateProof(3, 9, Height(20), loadHash(Height(20), 2)));

			auto pLastProof = utils::UniqueToShared(CreateProof(4, 16, Height(23), loadHash(Height(23), 3)));
			pProofStorage->setLastFinalizationProof(pLastProof);
			pProofStorage->saveProof(*pLastProof);

			context.boot(std::move(pProofStorage));

			// Act:
			auto heightHashPair = context.testState().state().hooks().localFinalizedHeightHashPairSupplier()();

			// Assert:
			auto expectedHash = context.testState().state().storage().view().loadBlockElement(localProofHeight)->EntityHash;
			EXPECT_EQ(model::HeightHashPair({ localProofHeight, expectedHash }), heightHashPair)
					<< "localProofHeight: " << localProofHeight
					<< "localChainHeight: " << localChainHeight;
		}
	}

	TEST(TEST_CLASS, LocalFinalizedHeightHashPairSupplierHookIsRegistered_NetworkFinalizedBlockMatches) {
		RunLocalFinalizedHeightHashPairSupplierHookTest(Height(23), 25);
		RunLocalFinalizedHeightHashPairSupplierHookTest(Height(23), 23); // remote proof height is equal to local chain height
	}

	TEST(TEST_CLASS, LocalFinalizedHeightHashPairSupplierHookIsRegistered_IntermediateFinalizedBlockMatches) {
		RunLocalFinalizedHeightHashPairSupplierHookTest(Height(10), 25);
		RunLocalFinalizedHeightHashPairSupplierHookTest(Height(10), 15); // remote proof height is greater than local chain height
	}

	TEST(TEST_CLASS, LocalFinalizedHeightHashPairSupplierHookIsRegistered_NemesisFinalizedBlockMatches) {
		RunLocalFinalizedHeightHashPairSupplierHookTest(Height(1), 25);
		RunLocalFinalizedHeightHashPairSupplierHookTest(Height(1), 9); // remote proof height is greater than local chain height
	}

	TEST(TEST_CLASS, NetworkFinalizedHeightHashPairSupplierHookIsRegistered) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();

		TestContext context;
		context.boot(FinalizationPoint(11), Height(123), hash);

		// Act:
		auto heightHashPair = context.testState().state().hooks().networkFinalizedHeightHashPairSupplier()();

		// Assert:
		EXPECT_EQ(model::HeightHashPair({ Height(123), hash }), heightHashPair);
	}

	// endregion

	// region FinalizationBootstrapperService - multi round message aggregator

	namespace {
		model::StepIdentifier CreateStepIdentifier(uint32_t point) {
			return test::CreateStepIdentifier(Finalization_Epoch.unwrap(), point, model::FinalizationStage::Prevote);
		}

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
			aggregator.modifier().add(context.createMessage(VoterType::Large1, CreateStepIdentifier(12), Height(22), hash));

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
			aggregator.modifier().setMaxFinalizationRound({ Finalization_Epoch, FinalizationPoint(15) });

			// Act:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			aggregator.modifier().add(context.createMessage(VoterType::Large1, CreateStepIdentifier(12), Height(22), hash));
			aggregator.modifier().add(context.createMessage(VoterType::Large1, CreateStepIdentifier(15), Height(24), hash));

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
			aggregator.modifier().setMaxFinalizationRound({ Finalization_Epoch, FinalizationPoint(15) });

			// Act:
			auto hash1 = test::GenerateRandomByteArray<Hash256>();
			aggregator.modifier().add(context.createMessage(VoterType::Large1, CreateStepIdentifier(12), Height(22), hash1));
			aggregator.modifier().add(context.createMessage(VoterType::Large2, CreateStepIdentifier(12), Height(22), hash1));

			auto hash2 = test::GenerateRandomByteArray<Hash256>();
			aggregator.modifier().add(context.createMessage(VoterType::Large1, CreateStepIdentifier(15), Height(24), hash2));
			aggregator.modifier().add(context.createMessage(VoterType::Large2, CreateStepIdentifier(15), Height(24), hash2));

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

	// region FinalizationBootstrapperService - proof storage

	TEST(TEST_CLASS, ProofStorageServiceDoesNotSupportPatchingFinalizedBlocks) {
		// Arrange:
		test::TempDirectoryGuard directoryGuard;

		TestContext context;
		mocks::SeedStorageWithFixedSizeBlocks(context.testState().state().storage(), 10);
		context.boot();

		const auto& userConfig = context.testState().state().config().User;
		const_cast<std::string&>(userConfig.DataDirectory) = directoryGuard.name();

		auto pPrevoteChainBlockStorage = mocks::CreateMemoryBlockStorageCache(7);
		auto proofRound = test::CreateFinalizationRound(7, 3);
		auto proofHash = pPrevoteChainBlockStorage->view().loadBlockElement(Height(7))->EntityHash;

		io::FilePrevoteChainStorage prevoteChainStorage(context.testState().state().config().User.DataDirectory);
		prevoteChainStorage.save(pPrevoteChainBlockStorage->view(), { proofRound, Height(5), 3 });

		auto pProof = model::CreateFinalizationProof({ proofRound, Height(7), proofHash }, {});

		// Act + Assert: does not throw even though block range consumer factory isn't registered
		auto& storage = GetProofStorageCache(context.locator());
		storage.modifier().saveProof(*pProof);
	}

	// endregion

	// region FinalizationBootstrapperPhaseTwoService - test context

	namespace {
		struct FinalizationBootstrapperPhaseTwoServiceTraits {
			static constexpr auto CreateRegistrar = CreateFinalizationBootstrapperPhaseTwoServiceRegistrar;
		};

		class PhaseTwoTestContext
				: public test::VoterSeededCacheDependentServiceLocatorTestContext<FinalizationBootstrapperPhaseTwoServiceTraits> {
		public:
			PhaseTwoTestContext() {
				// setup filesystem
				const auto& userConfig = testState().state().config().User;
				const_cast<std::string&>(userConfig.DataDirectory) = m_directoryGuard.name();

				// register dependent services
				auto hash = test::GenerateRandomByteArray<Hash256>();
				test::FinalizationBootstrapperServiceTestUtils::Register(
						locator(),
						testState().state(),
						std::make_unique<mocks::MockProofStorage>(Finalization_Epoch, FinalizationPoint(4), Height(1), hash));

				// setup hooks
				testState().state().hooks().setBlockRangeConsumerFactory([this](auto source) {
					return [this, source](auto&& blockRange) {
						m_capturedBlockRanges.emplace_back(source, std::move(blockRange));
					};
				});
			}

		public:
			const auto& capturedBlockRanges() {
				return m_capturedBlockRanges;
			}

		private:
			test::TempDirectoryGuard m_directoryGuard;

			std::vector<std::pair<disruptor::InputSource, model::AnnotatedEntityRange<model::Block>>> m_capturedBlockRanges;
		};
	}

	// endregion

	// region FinalizationBootstrapperPhaseTwoService - basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(FinalizationBootstrapperPhaseTwo, Post_Range_Consumers)

	TEST(PHASE_TWO_TEST_CLASS, NoServicesOrCountersAreRegistered) {
		// Arrange:
		PhaseTwoTestContext context;

		// Act:
		context.boot();

		// Assert: only phase one services are present
		EXPECT_EQ(Num_Services, context.locator().numServices());
		EXPECT_EQ(0u, context.locator().counters().size());
	}

	// endregion

	// region FinalizationBootstrapperPhaseTwoService - proof storage

	TEST(PHASE_TWO_TEST_CLASS, ProofStorageServiceSupportsPatchingFinalizedBlocks_MatchingBlock) {
		// Arrange:
		PhaseTwoTestContext context;
		mocks::SeedStorageWithFixedSizeBlocks(context.testState().state().storage(), 10);
		context.boot();

		auto proofRound = test::CreateFinalizationRound(7, 3);
		auto proofHash = context.testState().state().storage().view().loadBlockElement(Height(7))->EntityHash;

		io::FilePrevoteChainStorage prevoteChainStorage(context.testState().state().config().User.DataDirectory);
		prevoteChainStorage.save(context.testState().state().storage().view(), { proofRound, Height(5), 3 });

		auto pProof = model::CreateFinalizationProof({ proofRound, Height(7), proofHash }, {});

		// Act:
		auto& storage = GetProofStorageCache(context.locator());
		storage.modifier().saveProof(*pProof);

		// Assert:
		EXPECT_EQ(0u, context.capturedBlockRanges().size());
	}

	TEST(PHASE_TWO_TEST_CLASS, ProofStorageServiceSupportsPatchingFinalizedBlocks_MismatchedBlock) {
		// Arrange:
		PhaseTwoTestContext context;
		mocks::SeedStorageWithFixedSizeBlocks(context.testState().state().storage(), 10);
		context.boot();

		auto pPrevoteChainBlockStorage = mocks::CreateMemoryBlockStorageCache(7);
		auto proofRound = test::CreateFinalizationRound(7, 3);
		auto proofHash = pPrevoteChainBlockStorage->view().loadBlockElement(Height(7))->EntityHash;

		io::FilePrevoteChainStorage prevoteChainStorage(context.testState().state().config().User.DataDirectory);
		prevoteChainStorage.save(pPrevoteChainBlockStorage->view(), { proofRound, Height(5), 3 });

		auto pProof = model::CreateFinalizationProof({ proofRound, Height(7), proofHash }, {});

		// Act:
		auto& storage = GetProofStorageCache(context.locator());
		storage.modifier().saveProof(*pProof);

		// Assert:
		ASSERT_EQ(1u, context.capturedBlockRanges().size());
		EXPECT_EQ(disruptor::InputSource::Remote_Pull, context.capturedBlockRanges()[0].first);

		const auto& patchBlocksRange = context.capturedBlockRanges()[0].second;
		EXPECT_EQ(Key(), patchBlocksRange.SourceIdentity.PublicKey);
		EXPECT_EQ("", patchBlocksRange.SourceIdentity.Host);
		ASSERT_EQ(3u, patchBlocksRange.Range.size());

		auto patchBlocksIter = patchBlocksRange.Range.cbegin();
		for (auto i = 0u; i < 3; ++i, ++patchBlocksIter)
			EXPECT_EQ(*pPrevoteChainBlockStorage->view().loadBlock(Height(5 + i)), *patchBlocksIter) << i;
	}

	// endregion
}}
