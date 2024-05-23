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

#include "SecretLockInfoMapper.h"
#include "mongo/plugins/lock_shared/src/mappers/LockInfoMapper.h"

using namespace catapult::mongo::mappers;

namespace catapult {
namespace mongo {
	namespace plugins {

		namespace {
			class SecretLockInfoMapperTraits {
			public:
				using LockInfoType = state::SecretLockInfo;

			public:
				static void StreamLockInfo(bson_stream::document& builder, const state::SecretLockInfo& lockInfo) {
					builder << "hashAlgorithm" << utils::to_underlying_type(lockInfo.HashAlgorithm) << "secret" << ToBinary(lockInfo.Secret)
							<< "recipientAddress" << ToBinary(lockInfo.RecipientAddress) << "compositeHash" << ToBinary(lockInfo.CompositeHash);
				}
			};
		}

		bsoncxx::document::value ToDbModel(const state::SecretLockInfoHistory& history) {
			return LockInfoMapper<SecretLockInfoMapperTraits>::ToDbModel(history.back());
		}
	}
}
}
