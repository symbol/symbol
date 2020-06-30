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

#include "timesync/src/TimeSynchronizationUtils.h"
#include "timesync/src/CommunicationTimestamps.h"
#include "timesync/src/TimeSynchronizationConfiguration.h"
#include "timesync/src/TimeSynchronizationState.h"
#include "timesync/src/TimeSynchronizer.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/ionet/Node.h"
#include "timesync/tests/test/TimeSynchronizationCacheTestUtils.h"
#include "timesync/tests/test/TimeSynchronizationTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/TestHarness.h"
#include <limits>

namespace catapult { namespace timesync {

#define TEST_CLASS TimeSynchronizationUtilsTests

	namespace {
		constexpr int64_t Warning_Threshold_Millis = 5'000;
		constexpr Importance Total_Chain_Importance(1'000'000);
		constexpr uint64_t Default_Threshold = 85;
		constexpr auto Default_Epoch_Adjustment = utils::TimeSpan::FromMilliseconds(11223344556677);

		class SimpleResultSupplier {
		public:
			SimpleResultSupplier(std::vector<CommunicationTimestamps>&& communicationTimestampsContainer, size_t numValidNodes)
					: m_communicationTimestampsContainer(std::move(communicationTimestampsContainer))
					, m_numValidNodes(numValidNodes)
					, m_index(0)
			{}

		public:
			thread::future<TimeSyncRequestResultPair> operator()(const ionet::Node& node) {
				if (m_index >= m_communicationTimestampsContainer.size())
					CATAPULT_THROW_RUNTIME_ERROR("out of communication timestamps");

				m_capturedNodes.push_back(node);
				return m_index < m_numValidNodes
						? thread::make_ready_future(successResult())
						: thread::make_ready_future(failureResult());
			}

		private:
			TimeSyncRequestResultPair successResult() {
				return std::make_pair(net::NodeRequestResult::Success, m_communicationTimestampsContainer[m_index++]);
			}

			TimeSyncRequestResultPair failureResult() {
				return std::make_pair(net::NodeRequestResult::Failure_Timeout, m_communicationTimestampsContainer[m_index++]);
			}

		private:
			std::vector<CommunicationTimestamps> m_communicationTimestampsContainer;
			size_t m_numValidNodes;
			std::vector<ionet::Node> m_capturedNodes;
			size_t m_index;
		};

		class SimpleNetworkTimeSupplier {
		public:
			explicit SimpleNetworkTimeSupplier(std::vector<CommunicationTimestamps>&& communicationTimestampsContainer)
					: m_communicationTimestampsContainer(std::move(communicationTimestampsContainer))
					, m_index(0)
			{}

		public:
			Timestamp operator()() {
				if (m_index / 2 >= m_communicationTimestampsContainer.size())
					CATAPULT_THROW_RUNTIME_ERROR("out of network timestamps");

				return 0 == m_index % 2
						? m_communicationTimestampsContainer[m_index++ / 2].SendTimestamp
						: m_communicationTimestampsContainer[m_index++ / 2].ReceiveTimestamp;
			}

		private:
			std::vector<CommunicationTimestamps> m_communicationTimestampsContainer;
			size_t m_index;
		};

		enum class NodeType { Local, Remote };

		std::vector<CommunicationTimestamps> ExtractCommunicationTimestampsContainer(
				const std::vector<TimeSynchronizationSample>& samples,
				NodeType nodeType) {
			std::vector<CommunicationTimestamps> communicationTimestampsContainer;
			for (const auto& sample : samples) {
				communicationTimestampsContainer.push_back(NodeType::Local == nodeType
						? sample.localTimestamps()
						: sample.remoteTimestamps());
			}

			return communicationTimestampsContainer;
		}

		filters::AggregateSynchronizationFilter CreateEmptyAggregateFilter() {
			return filters::AggregateSynchronizationFilter({});
		}

		struct TestContext {
		public:
			explicit TestContext(
					const std::vector<TimeSynchronizationSample>& samples,
					size_t numValidNodes = std::numeric_limits<size_t>::max())
					: Synchronizer(CreateEmptyAggregateFilter(), Total_Chain_Importance, Warning_Threshold_Millis)
					, TimeSyncConfig(TimeSynchronizationConfiguration::Uninitialized())
					, RequestResultFutureSupplier(ExtractCommunicationTimestampsContainer(samples, NodeType::Remote), numValidNodes)
					, ServiceTestState(CreateCache())
					, pTimeSyncState(std::make_shared<TimeSynchronizationState>(Default_Epoch_Adjustment, Default_Threshold))
					, NetworkTimeSupplier(ExtractCommunicationTimestampsContainer(samples, NodeType::Local)) {
				TimeSyncConfig.MaxNodes = 5;

				auto& mutableBlockChainConfig = const_cast<model::BlockChainConfiguration&>(ServiceTestState.config().BlockChain);
				mutableBlockChainConfig.TotalChainImportance = Total_Chain_Importance;
			}

		public:
			TimeSynchronizer Synchronizer;
			TimeSynchronizationConfiguration TimeSyncConfig;
			SimpleResultSupplier RequestResultFutureSupplier;
			test::ServiceTestState ServiceTestState;
			std::shared_ptr<TimeSynchronizationState> pTimeSyncState;
			SimpleNetworkTimeSupplier NetworkTimeSupplier;

		private:
			static cache::CatapultCache CreateCache(Importance totalChainImportance) {
				auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
				blockChainConfig.ImportanceGrouping = 123;
				blockChainConfig.TotalChainImportance = totalChainImportance;
				return test::CoreSystemCacheFactory::Create(blockChainConfig);
			}

			static cache::CatapultCache CreateCache() {
				return CreateCache(Importance());
			}
		};

		thread::Task CreateTimeSyncTask(TestContext& context) {
			return CreateTimeSyncTask(
					context.Synchronizer,
					context.TimeSyncConfig,
					context.RequestResultFutureSupplier,
					context.ServiceTestState.state(),
					*context.pTimeSyncState,
					context.NetworkTimeSupplier);
		}
	}

	// region task - basic

	TEST(TEST_CLASS, CanCreateTask) {
		// Arrange:
		TestContext context({});

		// Act:
		auto task = CreateTimeSyncTask(context);

		// Assert:
		EXPECT_EQ("time synchronization task", task.Name);
	}

	TEST(TEST_CLASS, TaskExecutionIncreasesNodeAge) {
		// Arrange:
		TestContext context({});
		auto task = CreateTimeSyncTask(context);

		// Sanity:
		EXPECT_EQ(NodeAge(0), context.pTimeSyncState->nodeAge());

		// Act:
		for (auto i = 0u; i < 5; ++i) {
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
		}

		// Assert:
		EXPECT_EQ(NodeAge(5), context.pTimeSyncState->nodeAge());
	}

	// endregion

	// region task - time sync state update

	namespace {
		void SeedAccountStateCache(
				cache::AccountStateCacheDelta& delta,
				const std::vector<Key>& keys,
				const std::vector<Importance>& importances) {
			for (auto i = 0u; i < keys.size(); ++i)
				test::AddAccount(delta, keys[i], importances[i], model::ImportanceHeight(1));

			delta.updateHighValueAccounts(Height(1));
		}

		void SeedNodeContainer(ionet::NodeContainer& nodeContainer, const std::vector<Key>& keys) {
			auto i = 0u;
			for (const auto& key : keys) {
				auto nodeName = "Node" + std::to_string(++i);
				test::AddNode(nodeContainer, key, nodeName);
			}
		}

		timesync::TimeSynchronizationSample CreateTimeSyncSampleWithTimeOffset(int64_t timeOffset) {
			// zero roundtrip time is not realistic but identical send / receive timestamps are needed for tests since
			// the calls to the network time provider are not guaranteed to be in a deterministic order
			return timesync::TimeSynchronizationSample(
					test::GenerateRandomByteArray<Key>(),
					test::CreateCommunicationTimestamps(0, 0),
					test::CreateCommunicationTimestamps(timeOffset, timeOffset));
		}

		std::vector<TimeSynchronizationSample> CreateSamples(const std::vector<int64_t>& remoteOffsets) {
			std::vector<TimeSynchronizationSample> samples;
			for (auto offset : remoteOffsets)
				samples.push_back(CreateTimeSyncSampleWithTimeOffset(offset));

			return samples;
		}

		template<typename TAssertState>
		void AssertStateChange(
				const std::vector<int64_t>& remoteOffsets,
				const std::vector<Importance>& importances,
				TAssertState assertState) {
			// Arrange: prepare samples
			auto samples = CreateSamples(remoteOffsets);
			auto keys = test::ExtractKeys(samples);

			// - prepare context
			TestContext context(samples);
			SeedNodeContainer(context.ServiceTestState.state().nodes(), keys);

			// - prepare account state cache
			auto& cache = context.ServiceTestState.state().cache();
			{
				auto cacheDelta = cache.createDelta();
				SeedAccountStateCache(cacheDelta.sub<cache::AccountStateCache>(), keys, importances);
				cache.commit(Height(1));
			}

			// Sanity:
			EXPECT_EQ(TimeOffset(0), context.pTimeSyncState->offset());
			EXPECT_EQ(TimeOffsetDirection::Positive, context.pTimeSyncState->offsetDirection());
			EXPECT_EQ(NodeAge(0), context.pTimeSyncState->nodeAge());

			// Act:
			auto task = CreateTimeSyncTask(context);
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			assertState(*context.pTimeSyncState);
		}
	}

	TEST(TEST_CLASS, TaskProcessesSamples_Single) {
		AssertStateChange({ 250 }, { Importance(1'000'000) }, [](const auto& timeSyncState) {
			EXPECT_EQ(TimeOffset(250), timeSyncState.offset());
			EXPECT_EQ(NodeAge(1), timeSyncState.nodeAge());
		});
	}

	TEST(TEST_CLASS, TaskProcessesSamples_Multiple) {
		// Arrange:
		std::vector<int64_t> remoteOffsets{ 250, -150, 500, 0 };
		std::vector<Importance> importances{ Importance(250'000), Importance(250'000), Importance(250'000), Importance(250'000) };

		// Assert:
		AssertStateChange(remoteOffsets, importances, [](const auto& timeSyncState) {
			EXPECT_EQ(TimeOffset(150), timeSyncState.offset());
			EXPECT_EQ(NodeAge(1), timeSyncState.nodeAge());
		});
	}

	// endregion

	// region retrieve samples

	namespace {
		auto OrderSamples(const std::vector<TimeSynchronizationSample>& samples, const ionet::NodeSet& nodes) {
			std::vector<TimeSynchronizationSample> orderedSamples;
			for (const auto& node : nodes) {
				auto iter = std::find_if(samples.cbegin(), samples.cend(), [node](const auto& sample) {
					return sample.identityKey() == node.identity().PublicKey;
				});
				orderedSamples.push_back(*iter);
			}

			return orderedSamples;
		}

		void AssertRetrievedSamples(
				const std::vector<TimeSynchronizationSample>& originalSamples,
				const std::vector<size_t> expectedSampleIndexes,
				size_t numValidNodes = std::numeric_limits<size_t>::max()) {
			// Arrange:
			auto keys = test::ExtractKeys(originalSamples);

			ionet::NodeSet nodes;
			for (const auto& key : keys)
				nodes.insert(ionet::Node({ key, "11.22.33.44" }));

			// - need to reorder the samples to match the order of the nodes
			auto orderedSamples = OrderSamples(originalSamples, nodes);

			// - TestContext initializes SamplesResultsFutureSupplier and NetworkTimeSupplier to return samples
			//   with timestamps taken from orderedSamples
			TestContext context(orderedSamples, numValidNodes);

			// Act:
			thread::future<TimeSynchronizationSamples> samplesFuture;
			{
				samplesFuture = RetrieveSamples(nodes, context.RequestResultFutureSupplier, context.NetworkTimeSupplier);
			}

			auto samples = samplesFuture.get();

			// Assert:
			EXPECT_EQ(expectedSampleIndexes.size(), samples.size());
			auto i = 0;
			for (auto index : expectedSampleIndexes) {
				auto message = "at index " + std::to_string(i);
				auto& sample = orderedSamples[index];
				auto iter = samples.find(sample);

				ASSERT_TRUE(samples.cend() != iter) << message;
				EXPECT_EQ(sample, *iter) << message;
				++i;
			}
		}
	}

	TEST(TEST_CLASS, CanRetrieveSamples_Zero) {
		// Arrange:
		auto originalSamples = CreateSamples({});

		// Assert:
		AssertRetrievedSamples(originalSamples, {});
	}

	TEST(TEST_CLASS, CanRetrieveSamples_Single) {
		// Arrange:
		auto originalSamples = CreateSamples({ 100 });

		// Assert:
		AssertRetrievedSamples(originalSamples, { 0 });
	}

	TEST(TEST_CLASS, CanRetrieveSamples_Multiple) {
		// Arrange:
		auto originalSamples = CreateSamples({ 100, 200, 50 });

		// Assert:
		AssertRetrievedSamples(originalSamples, { 0, 1, 2 });
	}

	TEST(TEST_CLASS, NodesWithInteractionFailureAreIgnored) {
		// Arrange:
		auto originalSamples = CreateSamples({ 100, 200, 50, 500, -75, 90 });

		// Assert:
		AssertRetrievedSamples(originalSamples, { 0, 1 }, 2);
	}

	// endregion
}}
