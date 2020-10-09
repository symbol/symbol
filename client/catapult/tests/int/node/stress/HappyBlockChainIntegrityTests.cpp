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

#include "sdk/src/extensions/TransactionExtensions.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/config/ValidateConfiguration.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/ChainScore.h"
#include "tests/int/node/stress/test/BlockChainBuilder.h"
#include "tests/int/node/stress/test/TransactionsBuilder.h"
#include "tests/int/node/test/LocalNodeRequestTestUtils.h"
#include "tests/int/node/test/LocalNodeTestContext.h"
#include "tests/test/nodeps/Logging.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread.hpp>

namespace catapult { namespace local {

#define TEST_CLASS HappyBlockChainIntegrityTests

	namespace {
		constexpr size_t Default_Network_Size = 10;
		constexpr uint32_t Max_Rollback_Blocks = 124;

		struct HappyLocalNodeTraits {
			static constexpr auto CountersToLocalNodeStats = test::CountersToBasicLocalNodeStats;
			static constexpr auto AddPluginExtensions = test::AddSimplePartnerPluginExtensions;
		};

		using NodeTestContext = test::LocalNodeTestContext<HappyLocalNodeTraits>;

		uint16_t GetPortForNode(uint32_t id) {
			return static_cast<uint16_t>(test::GetLocalHostPort() + 10 * (id + 1));
		}

		ionet::Node CreateNode(uint32_t id) {
			auto networkFingerprint = model::UniqueNetworkFingerprint(model::NetworkIdentifier::Private_Test);
			auto metadata = ionet::NodeMetadata(networkFingerprint, "NODE " + std::to_string(id));
			metadata.Roles = ionet::NodeRoles::Peer;
			return ionet::Node(
					{ crypto::KeyPair::FromString(test::Test_Network_Private_Keys[id]).publicKey(), std::to_string(id) },
					test::CreateLocalHostNodeEndpoint(GetPortForNode(id)),
					metadata);
		}

		std::vector<ionet::Node> CreateNodes(size_t numNodes) {
			std::vector<ionet::Node> nodes;
			for (auto i = 0u; i < numNodes; ++i)
				nodes.push_back(CreateNode(i));

			return nodes;
		}

		void UpdateBlockChainConfiguration(model::BlockChainConfiguration& blockChainConfig) {
			blockChainConfig.ImportanceGrouping = Max_Rollback_Blocks / 2 + 1;
			blockChainConfig.MaxRollbackBlocks = Max_Rollback_Blocks;
			blockChainConfig.MaxDifficultyBlocks = Max_Rollback_Blocks - 1;
		}

		void UpdateConfigurationForNode(config::CatapultConfiguration& config, uint32_t id) {
			// 1. give each node its own ports
			auto port = GetPortForNode(id);
			auto& nodeConfig = const_cast<config::NodeConfiguration&>(config.Node);
			nodeConfig.Port = port;

			// 2. specify custom network settings
			UpdateBlockChainConfiguration(const_cast<model::BlockChainConfiguration&>(config.BlockChain));

			// 3. ensure configuration is valid
			ValidateConfiguration(config);
		}

		void RescheduleTasks(const std::string resourcesDirectory) {
			namespace pt = boost::property_tree;

			auto configFilePath = (boost::filesystem::path(resourcesDirectory) / "config-task.properties").generic_string();

			pt::ptree properties;
			pt::read_ini(configFilePath, properties);

			// 1. reconnect more rapidly so nodes have a better chance to find each other
			properties.put("static node refresh task.startDelay", "125ms");
			properties.put("static node refresh task.minDelay", "300ms");
			properties.put("static node refresh task.maxDelay", "2000ms");
			properties.put("connect peers task for service Sync.startDelay", "250ms");
			properties.put("connect peers task for service Sync.repeatDelay", "300ms");

			// 2. run far more frequent sync rounds
			properties.put("synchronizer task.startDelay", "500ms");
			properties.put("synchronizer task.repeatDelay", "300ms");

			pt::write_ini(configFilePath, properties);
		}

		uint8_t RandomByteClamped(uint8_t max) {
			return static_cast<uint8_t>(test::RandomByte() * max / std::numeric_limits<uint8_t>::max());
		}

		struct ChainStatistics {
			model::ChainScore Score;
			Hash256 StateHash;
			catapult::Height Height;
		};

		ChainStatistics PushRandomBlockChainToNode(
				const ionet::Node& node,
				test::StateHashCalculator& stateHashCalculator,
				const std::string& resourcesPath,
				const test::BlockChainBuilder::BlockReceiptsHashCalculator& blockReceiptsHashCalculator,
				size_t numBlocks,
				utils::TimeSpan blockTimeInterval) {
			constexpr uint32_t Num_Accounts = 11;
			test::Accounts accounts(Num_Accounts);

			test::TransactionsBuilder transactionsBuilder(accounts);
			for (auto i = 0u; i < numBlocks; ++i) {
				// don't allow account 0 to be recipient because it is sender
				auto recipientId = RandomByteClamped(Num_Accounts - 2) + 1u;
				transactionsBuilder.addTransfer(0, recipientId, Amount(1'000'000));
			}

			auto blockChainConfig = test::CreatePrototypicalBlockChainConfiguration();
			UpdateBlockChainConfiguration(blockChainConfig);

			test::BlockChainBuilder builder(accounts, stateHashCalculator, blockChainConfig, resourcesPath);
			builder.setBlockTimeInterval(blockTimeInterval);
			builder.setBlockReceiptsHashCalculator(blockReceiptsHashCalculator);
			auto blocks = builder.asBlockChain(transactionsBuilder);

			test::ExternalSourceConnection connection(node);
			test::PushEntities(connection, ionet::PacketType::Push_Block, blocks);

			const auto& lastBlock = *blocks.back();
			ChainStatistics chainStats;
			chainStats.Score = model::ChainScore(1); // initial nemesis chain score
			chainStats.StateHash = lastBlock.StateHash;
			chainStats.Height = lastBlock.Height;

			mocks::MockMemoryBlockStorage storage;
			auto pNemesisBlockElement = storage.loadBlockElement(Height(1));
			chainStats.Score += model::ChainScore(chain::CalculateScore(pNemesisBlockElement->Block, *blocks[0]));
			for (auto i = 0u; i < blocks.size() - 1; ++i)
				chainStats.Score += model::ChainScore(chain::CalculateScore(*blocks[i], *blocks[i + 1]));

			return chainStats;
		}

		struct HappyLocalNodeStatistics : public ChainStatistics {
			uint64_t NumActiveReaders;
			uint64_t NumActiveWriters;
		};

		HappyLocalNodeStatistics GetStatistics(const NodeTestContext& context) {
			const auto& localNodeStats = context.stats();
			const auto& cacheView = context.localNode().cache().createView();

			HappyLocalNodeStatistics stats;
			stats.Score = context.localNode().score();
			stats.StateHash = cacheView.calculateStateHash().StateHash;
			stats.Height = cacheView.height();
			stats.NumActiveReaders = localNodeStats.NumActiveReaders;
			stats.NumActiveWriters = localNodeStats.NumActiveWriters;
			return stats;
		}

		void LogStatistics(const ionet::Node& node, const ChainStatistics& stats) {
			CATAPULT_LOG(debug)
					<< "*** CHAIN STATISTICS FOR NODE: " << node << " ***" << std::endl
					<< " ------ score " << stats.Score << std::endl
					<< " - state hash " << stats.StateHash << std::endl
					<< " ----- height " << stats.Height;
		}

		void LogStatistics(const ionet::Node& node, const HappyLocalNodeStatistics& stats) {
			CATAPULT_LOG(debug)
					<< "*** STATISTICS FOR NODE: " << node << " ***" << std::endl
					<< " ------ score " << stats.Score << std::endl
					<< " - state hash " << stats.StateHash << std::endl
					<< " ----- height " << stats.Height << std::endl
					<< " ---- readers " << stats.NumActiveReaders << std::endl
					<< " ---- writers " << stats.NumActiveWriters;
		}

		// region network traits

		struct DenseNetworkTraits {
			static std::vector<ionet::Node> GetPeersForNode(uint32_t, const std::vector<ionet::Node>& networkNodes) {
				return networkNodes;
			}
		};

		struct SparseNetworkTraits {
			static std::vector<ionet::Node> GetPeersForNode(uint32_t id, const std::vector<ionet::Node>& networkNodes) {
				// let each node only pull from "next" node
				return { networkNodes[(id + 1) % networkNodes.size()] };
			}
		};

		// endregion

		// region block receipts traits

		class BlockReceiptsDisabledTraits {
		public:
			Hash256 calculateReceiptsHash(const model::Block&) const {
				return Hash256();
			}
		};

		class BlockReceiptsEnabledTraits {
		public:
			Hash256 calculateReceiptsHash(const model::Block& block) const {
				// happy block chain tests send transfers, so only harvest fee receipt needs to be added
				auto totalFee = model::CalculateBlockTransactionsInfo(block).TotalFee;

				model::BlockStatementBuilder blockStatementBuilder;
				model::BalanceChangeReceipt receipt(
						model::Receipt_Type_Harvest_Fee,
						model::GetSignerAddress(block),
						test::Default_Currency_Mosaic_Id,
						totalFee);
				blockStatementBuilder.addReceipt(receipt);

				auto pStatement = blockStatementBuilder.build();
				return model::CalculateMerkleHash(*pStatement);
			}
		};

		// endregion

		// region state hash traits

		class StateHashDisabledTraits {
		public:
#if defined __APPLE__
			static constexpr size_t Dense_Network_Size = 8;
			static constexpr size_t Sparse_Network_Size = Default_Network_Size;
#else
			static constexpr size_t Dense_Network_Size = Default_Network_Size;
			static constexpr size_t Sparse_Network_Size = Default_Network_Size;
#endif

		public:
			test::StateHashCalculator createStateHashCalculator(const NodeTestContext&, size_t) const {
				return test::StateHashCalculator();
			}
		};

		class StateHashEnabledTraits {
		public:
#if defined __APPLE__
			static constexpr size_t Dense_Network_Size = 4;
			static constexpr size_t Sparse_Network_Size = 4;
#else
			static constexpr size_t Dense_Network_Size = Default_Network_Size;
			static constexpr size_t Sparse_Network_Size = Default_Network_Size;
#endif

			static constexpr auto State_Hash_Directory = "statehash";

		public:
			// State_Hash_Directory is containing directory of all isolated directories used for state hash calculation
			StateHashEnabledTraits() : m_stateHashCalculationDir(State_Hash_Directory)
			{}

		public:
			test::StateHashCalculator createStateHashCalculator(const NodeTestContext& context, size_t id) const {
				boost::filesystem::path stateHashDirectory = m_stateHashCalculationDir.name();
				stateHashDirectory /= std::to_string(id);
				boost::filesystem::create_directories(stateHashDirectory);

				return test::StateHashCalculator(context.prepareFreshDataDirectory(stateHashDirectory.generic_string()));
			}

		private:
			test::TempDirectoryGuard m_stateHashCalculationDir;
		};

		// endregion

		// region consensus test

		template<typename TNetworkTraits, typename TVerifyTraits>
		void AssertMultiNodeNetworkCanReachConsensus(TVerifyTraits&& verifyTraits, size_t networkSize) {
			// Arrange: create nodes
			test::GlobalLogFilter testLogFilter(utils::LogLevel::debug);
			auto networkNodes = CreateNodes(networkSize);

			// Act: boot all nodes
			CATAPULT_LOG(debug) << "booting nodes";

			std::vector<std::unique_ptr<NodeTestContext>> contexts;
			std::vector<ChainStatistics> chainStatsPerNode;
			chainStatsPerNode.resize(networkSize);
			ChainStatistics bestChainStats;
			for (auto i = 0u; i < networkSize; ++i) {
				// - give each node a separate directory
				auto nodeFlag = test::NodeFlag::Require_Explicit_Boot | TVerifyTraits::Node_Flag;
				auto peers = TNetworkTraits::GetPeersForNode(i, networkNodes);
				auto configTransform = [i](auto& config) {
					UpdateConfigurationForNode(config, i);
					const_cast<config::NodeConfiguration&>(config.Node).OutgoingConnections.MaxConnections = 20;
				};
				auto postfix = "_" + std::to_string(i);
				contexts.push_back(std::make_unique<NodeTestContext>(nodeFlag, peers, configTransform, postfix));

				auto& context = *contexts.back();
				context.regenerateCertificates(crypto::KeyPair::FromString(test::Test_Network_Private_Keys[i]));

				// - (re)schedule a few tasks and boot the node
				RescheduleTasks(context.resourcesDirectory());
				context.boot();

				// - push a random number of different (valid) blocks to each node
				// - vary time spacing so that all chains will have different scores
				auto numBlocks = RandomByteClamped(Max_Rollback_Blocks - 1) + 1u; // always generate at least one block

				// - when stateHashCalculator data directory is empty, there is no cache lock so node resources can be used directly
				CATAPULT_LOG(debug) << "pushing initial chain to node " << i;
				auto stateHashCalculator = verifyTraits.createStateHashCalculator(context, i);
				auto chainStats = PushRandomBlockChainToNode(
						networkNodes[i],
						stateHashCalculator,
						stateHashCalculator.dataDirectory().empty() ? context.dataDirectory() : stateHashCalculator.dataDirectory(),
						[&verifyTraits](const auto& block) { return verifyTraits.calculateReceiptsHash(block); },
						numBlocks,
						utils::TimeSpan::FromSeconds(60 + i));

				// - wait for the first block element to get processed
				//   note that this is needed because else the node is shut down before processing the initial chain
				WAIT_FOR_VALUE_EXPR_SECONDS(1u, test::GetCounterValue(context.localNode().counters(), "BLK ELEM TOT"), 30);
				WAIT_FOR_VALUE_EXPR_SECONDS(0u, test::GetCounterValue(context.localNode().counters(), "BLK ELEM ACT"), 30);

				chainStatsPerNode[i] = chainStats;
				LogStatistics(networkNodes[i], chainStats);

				// reset context
				context.reset();
			}

			for (const auto& chainStats : chainStatsPerNode) {
				if (chainStats.Score > bestChainStats.Score)
					bestChainStats = chainStats;
			}

			// - boot network again
			for (auto i = 0u; i < networkSize; ++i) {
				contexts[i]->boot();
				CATAPULT_LOG(debug) << "node " << i << " rebooted";
			}

			// Assert: wait for nodes to sync among themselves
			for (auto i = 0u; i < networkSize; ++i) {
				const auto& node = networkNodes[i];
				const auto& context = *contexts[i];

				CATAPULT_LOG(debug) << "waiting for node " << node << " to get best chain (score = " << bestChainStats.Score << ")";
				LogStatistics(node, GetStatistics(context));

				try {
					// - block chain sync consumer updates score and then cache, so need to wait for both to avoid race condition
					const auto& cache = context.localNode().cache();
					WAIT_FOR_VALUE_EXPR_SECONDS(bestChainStats.Score, context.localNode().score(), 45);
					WAIT_FOR_VALUE_EXPR_SECONDS(bestChainStats.Height, cache.createView().height(), 10);
					WAIT_FOR_VALUE_EXPR_SECONDS(bestChainStats.StateHash, cache.createView().calculateStateHash().StateHash, 10);

					const auto& stats = GetStatistics(context);
					LogStatistics(node, stats);

					// - nodes have shared state
					EXPECT_EQ(bestChainStats.Score, stats.Score);
					EXPECT_EQ(bestChainStats.StateHash, stats.StateHash);
					EXPECT_EQ(bestChainStats.Height, stats.Height);
				} catch (const catapult_runtime_error&) {
					// - log bit more information on failure
					LogStatistics(node, GetStatistics(context));
					throw;
				}
			}
		}

		// endregion

		// region verify traits

		class VerifyNoneTraits : public StateHashDisabledTraits, public BlockReceiptsDisabledTraits {
		public:
			static constexpr auto Node_Flag = test::NodeFlag::Regular;
		};

		class VerifyReceiptsTraits : public StateHashDisabledTraits, public BlockReceiptsEnabledTraits {
		public:
			static constexpr auto Node_Flag = test::NodeFlag::Verify_Receipts;
		};

		class VerifyStateTraits : public StateHashEnabledTraits, public BlockReceiptsDisabledTraits {
		public:
			static constexpr auto Node_Flag = test::NodeFlag::Verify_State;
		};

		class VerifyAllTraits : public StateHashEnabledTraits, public BlockReceiptsEnabledTraits {
		public:
			static constexpr auto Node_Flag = test::NodeFlag::Verify_State | test::NodeFlag::Verify_Receipts;
		};

		// endregion
	}

#define VERIFY_OPTIONS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	NO_STRESS_TEST(TEST_CLASS, TEST_NAME##_VerifyNone) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VerifyNoneTraits>(); } \
	NO_STRESS_TEST(TEST_CLASS, TEST_NAME##_VerifyReceipts) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VerifyReceiptsTraits>(); } \
	NO_STRESS_TEST(TEST_CLASS, TEST_NAME##_VerifyState) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VerifyStateTraits>(); } \
	NO_STRESS_TEST(TEST_CLASS, TEST_NAME##_VerifyAll) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VerifyAllTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	VERIFY_OPTIONS_BASED_TEST(MultiNodeDenseNetworkCanReachConsensus) {
		AssertMultiNodeNetworkCanReachConsensus<DenseNetworkTraits>(TTraits(), TTraits::Dense_Network_Size);
	}

	VERIFY_OPTIONS_BASED_TEST(MultiNodeSparseNetworkCanReachConsensus) {
		AssertMultiNodeNetworkCanReachConsensus<SparseNetworkTraits>(TTraits(), TTraits::Sparse_Network_Size);
	}
}}
