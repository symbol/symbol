/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
	namespace config { class CatapultConfiguration; }
	namespace extensions { class LocalNodeChainScore; }
	namespace io { class BlockStorageCache; }
}

namespace catapult { namespace extensions {

	/// Reference to a local node's basic state.
	struct LocalNodeStateRef {
	public:
		/// Creates a local node state ref referencing state composed of
		/// \a config, \a cache, \a storage and \a score.
		LocalNodeStateRef(
				const config::CatapultConfiguration& config,
				cache::CatapultCache& cache,
				io::BlockStorageCache& storage,
				LocalNodeChainScore& score)
				: Config(config)
				, Cache(cache)
				, Storage(storage)
				, Score(score)
		{}

	public:
		/// Catapult configuration.
		const config::CatapultConfiguration& Config;

		/// Local node cache.
		cache::CatapultCache& Cache;

		/// Local node storage.
		io::BlockStorageCache& Storage;

		/// Local node score.
		LocalNodeChainScore& Score;
	};
}}
