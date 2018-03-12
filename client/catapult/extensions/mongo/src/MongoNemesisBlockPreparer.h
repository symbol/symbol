#pragma once

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace io {
		class BlockStorage;
		class LightBlockStorage;
	}
	namespace model { struct BlockChainConfiguration; }
	namespace mongo { class ExternalCacheStorage; }
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace mongo {

	/// Prepares a mongo nemesis block.
	class MongoNemesisBlockPreparer {
	public:
		/// Creates a preparer that will prepare destination (mongo) block storage (\a destinationStorage) and (mongo) cache
		/// storage (\a externalCacheStorage) using \a config, source block storage (\a sourceStorage) and \a pluginManager.
		MongoNemesisBlockPreparer(
				io::LightBlockStorage& destinationStorage,
				ExternalCacheStorage& externalCacheStorage,
				const model::BlockChainConfiguration& config,
				const io::BlockStorage& sourceStorage,
				const plugins::PluginManager& pluginManager);

	public:
		/// Prepares the mongo nemesis block given \a cache with all registered plugins.
		bool prepare(const cache::CatapultCache& cache) const ;

	private:
		io::LightBlockStorage& m_destinationStorage;
		ExternalCacheStorage& m_externalCacheStorage;
		const model::BlockChainConfiguration& m_config;
		const io::BlockStorage& m_sourceStorage;
		const plugins::PluginManager& m_pluginManager;
	};
}}
