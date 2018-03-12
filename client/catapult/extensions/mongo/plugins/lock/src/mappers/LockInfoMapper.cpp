#include "LockInfoMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock/src/model/LockInfo.h"
#include "catapult/utils/Casting.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	// region ToDbModel

	namespace {
		void StreamLockInfo(bson_stream::document& builder, const model::LockInfo& lockInfo, const Address& accountAddress) {
			builder
					<< "account" << ToBinary(lockInfo.Account)
					<< "accountAddress" << ToBinary(accountAddress)
					<< "mosaicId" << ToInt64(lockInfo.MosaicId)
					<< "amount" << ToInt64(lockInfo.Amount)
					<< "height" << ToInt64(lockInfo.Height)
					<< "status" << utils::to_underlying_type(lockInfo.Status);
		}
	}

	bsoncxx::document::value ToDbModel(const model::HashLockInfo& hashLockInfo, const Address& accountAddress) {
		bson_stream::document builder;
		auto doc = builder << "lock" << bson_stream::open_document;
		StreamLockInfo(builder, hashLockInfo, accountAddress);
		builder << "hash" << ToBinary(hashLockInfo.Hash);
		return doc
				<< bson_stream::close_document
				<< bson_stream::finalize;
	}

	bsoncxx::document::value ToDbModel(const model::SecretLockInfo& secretLockInfo, const Address& accountAddress) {
		bson_stream::document builder;
		auto doc = builder << "lock" << bson_stream::open_document;
		StreamLockInfo(builder, secretLockInfo, accountAddress);
		builder
				<< "hashAlgorithm" << utils::to_underlying_type(secretLockInfo.HashAlgorithm)
				<< "secret" << ToBinary(secretLockInfo.Secret)
				<< "recipient" << ToBinary(secretLockInfo.Recipient);
		return doc
				<< bson_stream::close_document
				<< bson_stream::finalize;
	}

	// endregion

	// region ToModel

	namespace {
		void ReadLockInfo(model::LockInfo& lockInfo, const bsoncxx::document::element dbLockInfo) {
			DbBinaryToModelArray(lockInfo.Account, dbLockInfo["account"].get_binary());
			lockInfo.MosaicId = GetValue64<MosaicId>(dbLockInfo["mosaicId"]);
			lockInfo.Amount = GetValue64<Amount>(dbLockInfo["amount"]);
			lockInfo.Height = GetValue64<Height>(dbLockInfo["height"]);
			lockInfo.Status = static_cast<model::LockStatus>(ToUint8(dbLockInfo["status"].get_int32()));
		}
	}

	void ToLockInfo(const bsoncxx::document::view& document, model::HashLockInfo& hashLockInfo) {
		auto dbLockInfo = document["lock"];
		ReadLockInfo(hashLockInfo, dbLockInfo);
		DbBinaryToModelArray(hashLockInfo.Hash, dbLockInfo["hash"].get_binary());
	}

	void ToLockInfo(const bsoncxx::document::view& document, model::SecretLockInfo& secretLockInfo) {
		auto dbLockInfo = document["lock"];
		ReadLockInfo(secretLockInfo, dbLockInfo);
		secretLockInfo.HashAlgorithm = static_cast<model::LockHashAlgorithm>(ToUint8(dbLockInfo["hashAlgorithm"].get_int32()));
		DbBinaryToModelArray(secretLockInfo.Secret, dbLockInfo["secret"].get_binary());
		DbBinaryToModelArray(secretLockInfo.Recipient, dbLockInfo["recipient"].get_binary());
	}

	// endregion
}}}
