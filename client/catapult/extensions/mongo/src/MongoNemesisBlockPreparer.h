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
