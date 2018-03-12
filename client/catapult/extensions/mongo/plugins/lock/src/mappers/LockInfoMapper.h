#pragma once
#include "mongo/src/mappers/MapperInclude.h"
#include "plugins/txes/lock/src/model/LockInfo.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Maps a \a hashLockInfo and \a accountAddress to the corresponding db model value.
	bsoncxx::document::value ToDbModel(const model::HashLockInfo& hashLockInfo, const Address& accountAddress);

	/// Maps a \a secretLockInfo and \a accountAddress to the corresponding db model value.
	bsoncxx::document::value ToDbModel(const model::SecretLockInfo& secretLockInfo, const Address& accountAddress);

	/// Maps a database \a document to \a hashLockInfo.
	void ToLockInfo(const bsoncxx::document::view& document, model::HashLockInfo& hashLockInfo);

	/// Maps a database \a document to \a secretLockInfo.
	void ToLockInfo(const bsoncxx::document::view& document, model::SecretLockInfo& secretLockInfo);
}}}
