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
