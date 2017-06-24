#include "LocalNodeTestUtils.h"

namespace catapult { namespace test {

	// region connection tests

	ExternalConnection CreateExternalConnection(unsigned short port) {
		ExternalConnection connection;
		connection.pPool = CreateStartedIoServiceThreadPool(1);

		auto serverKeyPair = LoadServerKeyPair();
		connection.pIo = ConnectToLocalHost(connection.pPool->service(), port, serverKeyPair.publicKey());
		return connection;
	}

	// endregion

	// region counter -> stats adapter

	namespace {
		bool HasName(const local::LocalNodeCounterValue& counter, const std::string& name) {
			return name == counter.id().name();
		}

		void CountersToBasicLocalNodeStats(const local::LocalNodeCounterValues& counters, BasicLocalNodeStats& stats) {
			stats.NumActiveReaders = GetCounterValue(counters, "READERS");
			stats.NumActiveWriters = GetCounterValue(counters, "WRITERS");
			stats.NumScheduledTasks = GetCounterValue(counters, "TASKS");
			stats.NumAddedBlockElements = GetCounterValue(counters, "BLK ELEMENTS");
			stats.NumAddedTransactionElements = GetCounterValue(counters, "TX ELEMENTS");
		}
	}

	bool HasCounter(const local::LocalNodeCounterValues& counters, const std::string& name) {
		return std::any_of(counters.cbegin(), counters.cend(), [&name](const auto& counter) { return HasName(counter, name); });
	}

	uint64_t GetCounterValue(const local::LocalNodeCounterValues& counters, const std::string& name) {
		for (const auto& counter : counters) {
			if (HasName(counter, name))
				return counter.value();
		}

		CATAPULT_THROW_INVALID_ARGUMENT_1("could not find counter with name", name);
	}

	BasicLocalNodeStats CountersToBasicLocalNodeStats(const local::LocalNodeCounterValues& counters) {
		BasicLocalNodeStats stats;
		CountersToBasicLocalNodeStats(counters, stats);
		return stats;
	}

	PeerLocalNodeStats CountersToPeerLocalNodeStats(const local::LocalNodeCounterValues& counters) {
		PeerLocalNodeStats stats;
		CountersToBasicLocalNodeStats(counters, stats);
		stats.NumActiveBroadcastWriters = GetCounterValue(counters, "B WRITERS");
		stats.NumUnlockedAccounts = GetCounterValue(counters, "UNLKED ACCTS");
		return stats;
	}

	// endregion
}}
