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
#include "mongo/src/ExternalCacheStorage.h"
#include <memory>

namespace catapult { namespace mongo { class MongoDatabase; } }

namespace catapult { namespace mongo { namespace storages {

	/// Creates a mongo block difficulty cache storage around \a database and \a difficultyHistorySize.
	std::unique_ptr<ExternalCacheStorage> CreateMongoBlockDifficultyCacheStorage(MongoDatabase&& database, uint64_t difficultyHistorySize);
}}}
