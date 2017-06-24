#pragma once
#include "UnconfirmedTransactionsCacheViewProvider.h"
#include "catapult/local/ConsumerFactory.h"
#include "catapult/local/LocalNodeStateRef.h"
#include "catapult/utils/NonCopyable.h"

namespace catapult {
	namespace net { class PacketWriters; }
	namespace thread {
		class MultiServicePool;
		class Scheduler;
		struct Task;
	}
}

namespace catapult { namespace local { namespace api {

	/// A service for managing api scheduled tasks.
	struct SchedulerService : public utils::NonCopyable {
	public:
		/// Creates a new api scheduler service around state composed of \a stateRef, \a transactionRegistry,
		/// and \a unconfirmedTransactionsCacheViewProvider.
		/// \a completionAwareBlockRangeConsumerFactory is used to configure block range processing.
		explicit SchedulerService(
				const LocalNodeStateConstRef& stateRef,
				const model::TransactionRegistry& transactionRegistry,
				const UnconfirmedTransactionsCacheViewProvider& unconfirmedTransactionsCacheViewProvider,
				const CompletionAwareBlockRangeConsumerFactoryFunc& completionAwareBlockRangeConsumerFactory);

	public:
		/// Returns the total number of scheduled tasks.
		size_t numScheduledTasks() const;

	public:
		/// Boots the service using \a pool parallelization with \a packetWriters.
		void boot(thread::MultiServicePool& pool, net::PacketWriters& packetWriters);

	private:
		thread::Task createConnectPeersTask(net::PacketWriters& packetWriters);

		thread::Task createSynchronizerTask(net::PacketWriters& packetWriters);

	private:
		// state
		LocalNodeStateConstRef m_stateRef;
		const model::TransactionRegistry& m_transactionRegistry;
		UnconfirmedTransactionsCacheViewProvider m_unconfirmedTransactionsCacheViewProvider;
		CompletionAwareBlockRangeConsumerFactoryFunc m_completionAwareBlockRangeConsumerFactory;

		// services
		std::weak_ptr<thread::Scheduler> m_pScheduler;
	};
}}}
