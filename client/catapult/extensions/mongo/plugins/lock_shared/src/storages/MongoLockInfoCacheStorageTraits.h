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
#include "mongo/src/storages/MongoCacheStorage.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Mongo lock info cache storage traits.
	template<typename TLockInfoTraits>
	struct MongoLockInfoCacheStorageTraits : public TLockInfoTraits {
	public:
		using CacheDeltaType = typename TLockInfoTraits::CacheDeltaType;
		using KeyType = typename TLockInfoTraits::KeyType;
		using ModelType = typename TLockInfoTraits::ModelType;

	public:
		/// Maps \a key to a mongo identifier.
		static auto MapToMongoId(const KeyType& key) {
			return mappers::ToBinary(key);
		}

		/// Maps \a lockInfo to a mongo document.
		static auto MapToMongoDocument(const ModelType& lockInfo, model::NetworkIdentifier) {
			return plugins::ToDbModel(lockInfo);
		}
	};
}}}
