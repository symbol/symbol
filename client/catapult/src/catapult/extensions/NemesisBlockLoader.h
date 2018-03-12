#pragma once
#include "catapult/functions.h"

namespace catapult {
	namespace cache { class CatapultCacheDelta; }
	namespace extensions { struct LocalNodeStateRef; }
	namespace model {
		struct BlockChainConfiguration;
		struct BlockElement;
		class NotificationPublisher;
		class TransactionRegistry;
	}
	namespace observers { class EntityObserver; }
	namespace state { struct CatapultState; }
}

namespace catapult { namespace extensions {

	/// Loads and executes a nemesis block.
	class NemesisBlockLoader {
	public:
		/// Creates a loader around \a transactionRegistry, \a publisher and \a observer.
		NemesisBlockLoader(
				const model::TransactionRegistry& transactionRegistry,
				const model::NotificationPublisher& publisher,
				const observers::EntityObserver& observer);

	public:
		/// Loads the nemesis block from storage, updates state in \a stateRef and commits all changes.
		void executeAndCommit(const LocalNodeStateRef& stateRef) const;

		/// Executes the nemesis block (\a nemesisBlockElement), applies all changes to \a cacheDelta and checks consistency
		/// against \a config.
		/// \note Execution uses a default catapult state.
		void execute(
				const model::BlockChainConfiguration& config,
				const model::BlockElement& nemesisBlockElement,
				cache::CatapultCacheDelta& cacheDelta) const;

	private:
		enum class Verbosity { Off, On };

		void execute(
				const model::BlockChainConfiguration& config,
				const model::BlockElement& nemesisBlockElement,
				cache::CatapultCacheDelta& cacheDelta,
				state::CatapultState& catapultState,
				Verbosity verbosity) const;

	private:
		const model::TransactionRegistry& m_transactionRegistry;
		const model::NotificationPublisher& m_publisher;
		const observers::EntityObserver& m_observer;
	};
}}
