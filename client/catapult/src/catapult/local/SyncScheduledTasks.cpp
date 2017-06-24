#include "SyncScheduledTasks.h"
#include "catapult/chain/RemoteApi.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/thread/FutureUtils.h"

namespace catapult { namespace local {

	namespace {
		CPP14_CONSTEXPR
		utils::LogLevel MapToLogLevel(net::PeerConnectResult connectResult) {
			switch (connectResult) {
			case net::PeerConnectResult::Accepted:
				return utils::LogLevel::Info;

			case net::PeerConnectResult::Already_Connected:
				return utils::LogLevel::Trace;

			default:
				return utils::LogLevel::Warning;
			}
		}
	}

	thread::Task CreateConnectPeersTask(const std::vector<ionet::Node>& peers, net::PacketWriters& packetWriters) {
		thread::Task task;
		task.StartDelay = utils::TimeSpan::FromMilliseconds(10);
		task.RepeatDelay = utils::TimeSpan::FromMinutes(1);
		task.Name = "connect peers task";

		task.Callback = [peers, &packetWriters]() {
			if (peers.empty()) {
				CATAPULT_LOG(warning) << "could not find any peers for connecting";
				return thread::make_ready_future(thread::TaskResult::Continue);
			}

			std::vector<thread::future<bool>> futures;
			futures.reserve(peers.size());
			for (const auto& node : peers) {
				auto pPromise = std::make_shared<thread::promise<bool>>();
				futures.push_back(pPromise->get_future());

				packetWriters.connect(node, [node, pPromise](auto connectResult) {
					pPromise->set_value(true);
					CATAPULT_LOG_LEVEL(MapToLogLevel(connectResult))
							<< "connection attempt to " << node << " completed with " << connectResult;
				});
			}

			return thread::when_all(std::move(futures)).then([](const auto&) { return thread::TaskResult::Continue; });
		};
		return task;
	}

	namespace {
		auto CreateChainSynchronizerConfiguration(const config::LocalNodeConfiguration& config) {
			chain::ChainSynchronizerConfiguration chainSynchronizerConfig;
			chainSynchronizerConfig.MaxBlocksPerSyncAttempt = config.Node.MaxBlocksPerSyncAttempt;
			chainSynchronizerConfig.MaxChainBytesPerSyncAttempt = config.Node.MaxChainBytesPerSyncAttempt.bytes32();
			chainSynchronizerConfig.MaxRollbackBlocks = config.BlockChain.MaxRollbackBlocks;
			return chainSynchronizerConfig;
		}
	}

	thread::Task CreateSynchronizerTask(
			const config::LocalNodeConfiguration& config,
			const io::BlockStorageCache& storage,
			const model::TransactionRegistry& transactionRegistry,
			net::PacketWriters& packetWriters,
			const api::ChainScoreSupplier& chainScoreSupplier,
			const chain::ShortHashesSupplier& shortHashesSupplier,
			const chain::CompletionAwareBlockRangeConsumerFunc& blockRangeConsumer,
			const chain::TransactionRangeConsumerFunc& transactionRangeConsumer) {
		thread::Task task;
		task.StartDelay = utils::TimeSpan::FromSeconds(3);
		task.RepeatDelay = utils::TimeSpan::FromSeconds(3);
		task.Name = "synchronizer task";

		auto chainSynchronizer = chain::CreateChainSynchronizer(
				api::CreateLocalChainApi(
						storage,
						chainScoreSupplier,
						config.Node.MaxBlocksPerSyncAttempt),
				CreateChainSynchronizerConfiguration(config),
				shortHashesSupplier,
				blockRangeConsumer,
				transactionRangeConsumer);

		task.Callback = [&transactionRegistry, &packetWriters, chainSynchronizer, timeout = config.Node.SyncTimeout]() {
			auto packetIoPair = packetWriters.pickOne(timeout);
			if (!packetIoPair) {
				CATAPULT_LOG(warning) << "could not find peer to sync with";
				return thread::make_ready_future(thread::TaskResult::Continue);
			}

			auto pRemoteApi = std::make_shared<chain::RemoteApi>(
					packetIoPair.io(),
					// pass in a non-owning pointer to the registry
					std::shared_ptr<const model::TransactionRegistry>(&transactionRegistry, [](const auto*) {}));
			return chainSynchronizer(*pRemoteApi).then([pRemoteApi, packetIoPair](auto&& resultFuture) {
				auto result = resultFuture.get();
				CATAPULT_LOG(debug) << "completed synchronizer task for [" << packetIoPair.node() << "] with " << result;
				return thread::TaskResult::Continue;
			});
		};
		return task;
	}
}}
