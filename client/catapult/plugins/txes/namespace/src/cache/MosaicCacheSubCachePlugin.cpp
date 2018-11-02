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

#include "MosaicCacheSubCachePlugin.h"

namespace catapult { namespace cache {

	void MosaicCacheSummaryCacheStorage::saveAll(io::OutputStream& output) const {
		auto view = cache().createView();
		io::Write64(output, view->deepSize());
		output.flush();
	}

	void MosaicCacheSummaryCacheStorage::loadAll(io::InputStream& input, size_t) {
		auto deepSize = io::Read64(input);
		cache().init(deepSize);
	}

	MosaicCacheSubCachePlugin::MosaicCacheSubCachePlugin(const CacheConfiguration& config)
			: BaseMosaicCacheSubCachePlugin(std::make_unique<MosaicCache>(config))
	{}
}}
