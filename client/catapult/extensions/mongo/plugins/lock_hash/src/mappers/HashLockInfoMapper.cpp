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

#include "HashLockInfoMapper.h"
#include "mongo/plugins/lock_shared/src/mappers/LockInfoMapper.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		class HashLockInfoMapperTraits {
		public:
			using LockInfoType = state::HashLockInfo;

		public:
			static void StreamLockInfo(bson_stream::document& builder, const state::HashLockInfo& hashLockInfo) {
				builder << "hash" << ToBinary(hashLockInfo.Hash);
			}

			static void ReadLockInfo(state::HashLockInfo& hashLockInfo, const bsoncxx::document::element dbLockInfo) {
				DbBinaryToModelArray(hashLockInfo.Hash, dbLockInfo["hash"].get_binary());
			}
		};
	}

	bsoncxx::document::value ToDbModel(const state::HashLockInfo& hashLockInfo, const Address& accountAddress) {
		return LockInfoMapper<HashLockInfoMapperTraits>::ToDbModel(hashLockInfo, accountAddress);
	}

	void ToLockInfo(const bsoncxx::document::view& document, state::HashLockInfo& hashLockInfo) {
		LockInfoMapper<HashLockInfoMapperTraits>::ToLockInfo(document, hashLockInfo);
	}
}}}
