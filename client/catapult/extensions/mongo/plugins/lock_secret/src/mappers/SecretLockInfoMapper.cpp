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

#include "SecretLockInfoMapper.h"
#include "mongo/plugins/lock_shared/src/mappers/LockInfoMapper.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		class SecretLockInfoMapperTraits {
		public:
			using LockInfoType = state::SecretLockInfo;

		public:
			static void StreamLockInfo(bson_stream::document& builder, const state::SecretLockInfo& secretLockInfo) {
				builder
						<< "hashAlgorithm" << utils::to_underlying_type(secretLockInfo.HashAlgorithm)
						<< "secret" << ToBinary(secretLockInfo.Secret)
						<< "recipient" << ToBinary(secretLockInfo.Recipient)
						<< "compositeHash" << ToBinary(secretLockInfo.CompositeHash);
			}

			static void ReadLockInfo(state::SecretLockInfo& secretLockInfo, const bsoncxx::document::element dbLockInfo) {
				secretLockInfo.HashAlgorithm = static_cast<model::LockHashAlgorithm>(ToUint8(dbLockInfo["hashAlgorithm"].get_int32()));
				DbBinaryToModelArray(secretLockInfo.Secret, dbLockInfo["secret"].get_binary());
				DbBinaryToModelArray(secretLockInfo.Recipient, dbLockInfo["recipient"].get_binary());
				DbBinaryToModelArray(secretLockInfo.CompositeHash, dbLockInfo["compositeHash"].get_binary());
			}
		};
	}

	bsoncxx::document::value ToDbModel(const state::SecretLockInfo& secretLockInfo, const Address& accountAddress) {
		return LockInfoMapper<SecretLockInfoMapperTraits>::ToDbModel(secretLockInfo, accountAddress);
	}

	void ToLockInfo(const bsoncxx::document::view& document, state::SecretLockInfo& secretLockInfo) {
		LockInfoMapper<SecretLockInfoMapperTraits>::ToLockInfo(document, secretLockInfo);
	}
}}}
