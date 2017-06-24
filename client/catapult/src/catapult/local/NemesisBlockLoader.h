#pragma once
#include <functional>

namespace catapult {
	namespace cache { class CatapultCacheDelta; }
	namespace local { struct LocalNodeStateRef; }
	namespace model { class TransactionRegistry; }
	namespace observers { class EntityObserver; }
}

namespace catapult { namespace local {

	/// Prototype for a function that is called directly before commit.
	using PreCommitFunc = std::function<void (cache::CatapultCacheDelta&)>;

	/// Loads the nemesis block from storage using the supplied \a registry and \a observer,
	/// updating \a stateRef and calling \a preCommit before commiting.
	void LoadNemesisBlock(
			const model::TransactionRegistry& registry,
			const observers::EntityObserver& observer,
			const LocalNodeStateRef& stateRef,
			const PreCommitFunc& preCommit = PreCommitFunc());
}}
