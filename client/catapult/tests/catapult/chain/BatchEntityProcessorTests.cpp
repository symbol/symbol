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

#include "catapult/chain/BatchEntityProcessor.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/other/MockExecutionConfiguration.h"
#include "tests/TestHarness.h"

using namespace catapult::validators;

namespace catapult { namespace chain {

#define TEST_CLASS BatchEntityProcessorTests

	namespace {
		class ProcessorTestContext {
		public:
			ProcessorTestContext() : m_processor(CreateBatchEntityProcessor(m_executionConfig.Config))
			{}

		public:
			const auto& statefulValidatorParams() const {
				return m_executionConfig.pValidator->params();
			}

			void setValidationResult(validators::ValidationResult result, size_t trigger) {
				m_executionConfig.pValidator->setResult(result, trigger);
			}

		public:
			ValidationResult process(Height height, Timestamp timestamp, const model::WeakEntityInfos& entityInfos) {
				auto cache = test::CreateCatapultCacheWithMarkerAccount();
				auto delta = cache.createDelta();
				auto observerState = observers::ObserverState(delta);
				return m_processor(height, timestamp, entityInfos, observerState);
			}

		public:
			// Asserts call counters.
			void assertCounters(
					size_t numExpectedPublisherCalls,
					size_t numExpectedValidatorCalls,
					size_t numExpectedObserverCalls) const {
				// Assert:
				EXPECT_EQ(numExpectedPublisherCalls, m_executionConfig.pNotificationPublisher->params().size());
				EXPECT_EQ(numExpectedValidatorCalls, statefulValidatorParams().size());
				EXPECT_EQ(numExpectedObserverCalls, m_executionConfig.pObserver->params().size());
			}

		private:
			void assertValidatorContexts(Height height, Timestamp timestamp) const {
				CATAPULT_LOG(debug) << "checking validator contexts passed to validator";
				size_t i = 0;
				for (const auto& params : m_executionConfig.pValidator->params()) {
					auto message = "validator at " + std::to_string(i);
					// - context (use resolver call to implicitly test creation of ResolverContext)
					EXPECT_EQ(height, params.Context.Height) << message;
					EXPECT_EQ(timestamp, params.Context.BlockTime) << message;
					EXPECT_EQ(test::Mock_Execution_Configuration_Network_Identifier, params.Context.Network.Identifier) << message;
					EXPECT_EQ(MosaicId(22), params.Context.Resolvers.resolve(UnresolvedMosaicId(11))) << message;

					// - cache contents + sequence (NumStatistics is incremented by each observer call)
					EXPECT_TRUE(params.IsPassedMarkedCache) << message;
					EXPECT_EQ(i, params.NumStatistics) << message;
					++i;
				}
			}

			void assertObserverContexts(Height height) const {
				CATAPULT_LOG(debug) << "checking observer contexts passed to observer";
				size_t i = 0;
				for (const auto& params : m_executionConfig.pObserver->params()) {
					auto message = "observer at " + std::to_string(i);
					// - context (use resolver call to implicitly test creation of ResolverContext)
					EXPECT_EQ(height, params.Context.Height) << message;
					EXPECT_EQ(observers::NotifyMode::Commit, params.Context.Mode) << message;
					EXPECT_EQ(MosaicId(22), params.Context.Resolvers.resolve(UnresolvedMosaicId(11))) << message;

					// - cache contents + sequence (NumStatistics is incremented by each observer call)
					EXPECT_TRUE(params.IsPassedMarkedCache) << message;
					EXPECT_EQ(i, params.NumStatistics) << message;
					++i;
				}
			}

		public:
			// Asserts contexts passed to validator and observer.
			void assertContexts(Height height, Timestamp timestamp) const {
				// Assert:
				assertValidatorContexts(height, timestamp);
				assertObserverContexts(height);
			}

		private:
			void assertPublisherEntities(const model::WeakEntityInfos& entityInfos) const {
				CATAPULT_LOG(debug) << "checking entities passed to publisher";
				const auto& publisherParams = m_executionConfig.pNotificationPublisher->params();
				for (auto i = 0u; i < publisherParams.size(); ++i) {
					// - publisher is called for each entity (1 entity : 1 publisher call)
					EXPECT_EQ(entityInfos[i], publisherParams[i].EntityInfo) << "publisher at " << i;
					EXPECT_EQ(entityInfos[i].hash(), publisherParams[i].HashCopy) << "publisher at " << i;
				}
			}

			void assertValidatorEntities(const model::WeakEntityInfos& entityInfos) const {
				CATAPULT_LOG(debug) << "checking entities passed to validator";
				const auto& validatorParams = m_executionConfig.pValidator->params();
				for (auto i = 0u; i < validatorParams.size(); ++i) {
					// - validator is called twice for each entity (1 entity : 2 validator calls)
					EXPECT_EQ(entityInfos[i / 2].hash(), validatorParams[i].HashCopy) << "validator at " << i;
					EXPECT_EQ(0 == i % 2 ? 1u : 2u, validatorParams[i].SequenceId) << "validator at " << i;
				}
			}

			void assertObserverEntities(const model::WeakEntityInfos& entityInfos) const {
				CATAPULT_LOG(debug) << "checking entities passed to observer";
				const auto& observerParams = m_executionConfig.pObserver->params();
				for (auto i = 0u; i < observerParams.size(); ++i) {
					// - observer is called twice for each entity (1 entity : 2 observer calls)
					EXPECT_EQ(entityInfos[i / 2].hash(), observerParams[i].HashCopy) << "observer at " << i;
					EXPECT_EQ(0 == i % 2 ? 1u : 2u, observerParams[i].SequenceId) << "observer at " << i;
				}
			}

		public:
			// Asserts entity infos passed to publisher, validator and observer.
			void assertEntityInfos(const model::WeakEntityInfos& entityInfos) const {
				// Assert:
				assertPublisherEntities(entityInfos);
				assertValidatorEntities(entityInfos);
				assertObserverEntities(entityInfos);
			}

		private:
			test::MockExecutionConfiguration m_executionConfig;
			BatchEntityProcessor m_processor;
		};

		model::WeakEntityInfos ExtractEntityInfosFromBlock(const model::Block& block) {
			// Arrange: extract all entities from block (block entity should be extracted last)
			std::vector<const model::VerifiableEntity*> entities;
			for (const auto& tx : block.Transactions())
				entities.push_back(&tx);

			entities.push_back(&block);

			// - map to entity infos (use the entity itself as its fake hash since we know it will be alive)
			model::WeakEntityInfos entityInfos;
			for (auto i = 0u; i < entities.size(); ++i)
				entityInfos.push_back(model::WeakEntityInfo(*entities[i], reinterpret_cast<const Hash256&>(*entities[i])));

			return entityInfos;
		}
	}

	TEST(TEST_CLASS, CanProcessZeroEntities) {
		// Arrange:
		ProcessorTestContext context;
		model::WeakEntityInfos entityInfos;

		// Act:
		auto result = context.process(Height(246), Timestamp(721), entityInfos);

		// Assert: since there are no entities, no publish calls should occur
		EXPECT_EQ(ValidationResult::Neutral, result);
		context.assertCounters(0, 0, 0);
	}

	TEST(TEST_CLASS, CanProcessSingleEntity) {
		// Arrange:
		ProcessorTestContext context;
		auto pBlock = test::GenerateBlockWithTransactions(0);
		auto entityInfos = ExtractEntityInfosFromBlock(*pBlock);

		// Act:
		auto result = context.process(Height(246), Timestamp(721), entityInfos);

		// Assert: each publish call creates two downstream notifications
		EXPECT_EQ(ValidationResult::Success, result);
		context.assertCounters(1, 2, 2);
		context.assertContexts(Height(246), Timestamp(721));
		context.assertEntityInfos(entityInfos);
	}

	TEST(TEST_CLASS, CanProcessMultipleEntities) {
		// Arrange:
		ProcessorTestContext context;
		auto pBlock = test::GenerateBlockWithTransactions(3);
		auto entityInfos = ExtractEntityInfosFromBlock(*pBlock);

		// Act:
		auto result = context.process(Height(247), Timestamp(723), entityInfos);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
		context.assertCounters(4, 8, 8);
		context.assertContexts(Height(247), Timestamp(723));
		context.assertEntityInfos(entityInfos);
	}

	namespace {
		void AssertValidatorContext(const validators::ValidatorContext& context, Height height, Timestamp blockTime) {
			EXPECT_EQ(height, context.Height);
			EXPECT_EQ(blockTime, context.BlockTime);
		}
	}

	TEST(TEST_CLASS, CanReuseProcessor) {
		// Arrange:
		ProcessorTestContext context;
		auto pBlock1 = test::GenerateBlockWithTransactions(0);
		auto pBlock2 = test::GenerateBlockWithTransactions(0);
		auto entityInfos1 = ExtractEntityInfosFromBlock(*pBlock1);
		auto entityInfos2 = ExtractEntityInfosFromBlock(*pBlock2);

		// Act:
		context.process(Height(246), Timestamp(723), entityInfos1);
		context.process(Height(250), Timestamp(777), entityInfos2);

		// Assert: two captures for each block (two notifications per entity)
		size_t i = 0;
		const auto& capturedParams = context.statefulValidatorParams();
		ASSERT_EQ(4u, capturedParams.size());
		AssertValidatorContext(capturedParams[i++].Context, Height(246), Timestamp(723));
		AssertValidatorContext(capturedParams[i++].Context, Height(246), Timestamp(723));
		AssertValidatorContext(capturedParams[i++].Context, Height(250), Timestamp(777));
		AssertValidatorContext(capturedParams[i++].Context, Height(250), Timestamp(777));
	}

#define SHORT_CIRCUIT_TRAITS_BASED_TEST(TEST_NAME) \
	template<ValidationResult TResult> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Neutral) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValidationResult::Neutral>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Failure) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValidationResult::Failure>(); } \
	template<ValidationResult TResult> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	SHORT_CIRCUIT_TRAITS_BASED_TEST(ExecuteShortCircuitsOnSingleEntityStatefulValidation) {
		// Arrange:
		ProcessorTestContext context;
		context.setValidationResult(TResult, 2);
		auto pBlock = test::GenerateBlockWithTransactions(3);
		auto entityInfos = ExtractEntityInfosFromBlock(*pBlock);

		// Act:
		auto result = context.process(Height(248), Timestamp(725), entityInfos);

		// Assert:
		// - single stateful validator returned { success, interrput }
		// - only one observer was called (after success, but not interrupt)
		EXPECT_EQ(TResult, result);
		context.assertCounters(1, 2, 1);
		context.assertContexts(Height(248), Timestamp(725));
		context.assertEntityInfos(entityInfos);
	}
}}
