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
#include "PropertyCacheDelta.h"
#include "PropertyCacheView.h"
#include "catapult/cache/BasicCache.h"
#include "catapult/model/NetworkInfo.h"

namespace catapult { namespace cache {

	/// Cache composed of property information.
	using BasicPropertyCache = BasicCache<PropertyCacheDescriptor, PropertyCacheTypes::BaseSets, model::NetworkIdentifier>;

	/// Synchronized cache composed of property information.
	class PropertyCache : public SynchronizedCache<BasicPropertyCache> {
	public:
		DEFINE_CACHE_CONSTANTS(Property)

	public:
		/// Creates a cache around \a config and \a networkIdentifier.
		explicit PropertyCache(const CacheConfiguration& config, model::NetworkIdentifier networkIdentifier)
				: SynchronizedCache<BasicPropertyCache>(BasicPropertyCache(config, std::move(networkIdentifier)))
		{}
	};
}}
