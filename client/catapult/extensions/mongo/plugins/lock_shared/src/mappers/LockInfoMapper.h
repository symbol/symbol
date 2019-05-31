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
#include "mongo/src/mappers/MapperInclude.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock_shared/src/state/LockInfo.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Traits based lock info mapper.
	template<typename TTraits>
	class LockInfoMapper {
	private:
		using LockInfoType = typename TTraits::LockInfoType;

		// region ToDbModel

	private:
		static void StreamLockMetadata(mappers::bson_stream::document& builder) {
			builder
					<< "meta"
					<< mappers::bson_stream::open_document
					<< mappers::bson_stream::close_document;
		}

		static void StreamLockInfo(
				mappers::bson_stream::document& builder,
				const state::LockInfo& lockInfo,
				const Address& accountAddress) {
			using namespace catapult::mongo::mappers;

			builder
					<< "account" << ToBinary(lockInfo.Account)
					<< "accountAddress" << ToBinary(accountAddress)
					<< "mosaicId" << ToInt64(lockInfo.MosaicId)
					<< "amount" << ToInt64(lockInfo.Amount)
					<< "height" << ToInt64(lockInfo.Height)
					<< "status" << utils::to_underlying_type(lockInfo.Status);
		}

	public:
		static bsoncxx::document::value ToDbModel(const LockInfoType& lockInfo, const Address& accountAddress) {
			// lock metadata
			mappers::bson_stream::document builder;
			StreamLockMetadata(builder);

			// lock data
			auto doc = builder << "lock" << mappers::bson_stream::open_document;
			StreamLockInfo(builder, lockInfo, accountAddress);
			TTraits::StreamLockInfo(builder, lockInfo);
			return doc
					<< mappers::bson_stream::close_document
					<< mappers::bson_stream::finalize;
		}

		// endregion

		// region ToModel

	private:
		static void ReadLockInfo(state::LockInfo& lockInfo, const bsoncxx::document::element dbLockInfo) {
			using namespace catapult::mongo::mappers;

			DbBinaryToModelArray(lockInfo.Account, dbLockInfo["account"].get_binary());
			lockInfo.MosaicId = GetValue64<MosaicId>(dbLockInfo["mosaicId"]);
			lockInfo.Amount = GetValue64<Amount>(dbLockInfo["amount"]);
			lockInfo.Height = GetValue64<Height>(dbLockInfo["height"]);
			lockInfo.Status = static_cast<state::LockStatus>(ToUint8(dbLockInfo["status"].get_int32()));
		}

	public:
		static void ToLockInfo(const bsoncxx::document::view& document, LockInfoType& lockInfo) {
			auto dbLockInfo = document["lock"];
			ReadLockInfo(lockInfo, dbLockInfo);
			TTraits::ReadLockInfo(lockInfo, dbLockInfo);
		}

		// endregion
	};
}}}
