#pragma once
#include "catapult/utils/NonCopyable.h"

namespace catapult {
	namespace extensions {
		struct LocalNodeStateConstRef;
		struct LocalNodeStateRef;
	}
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace extensions {

	/// An interface for saving and loading block chains.
	class BlockChainStorage : public utils::NonCopyable {
	public:
		virtual ~BlockChainStorage() {}

	public:
		/// Loads data from storage into \a stateRef using plugins registered with \a pluginManager.
		virtual void loadFromStorage(const LocalNodeStateRef& stateRef, const plugins::PluginManager& pluginManager) = 0;

		/// Saves all in memory state from \a stateRef to storage.
		virtual void saveToStorage(const LocalNodeStateConstRef& stateRef) = 0;
	};
}}
