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

#pragma once
#include "src/cache/SecretLockInfoCacheTypes.h"
#include "src/storages/MongoSecretLockInfoCacheStorage.h"
#include "tests/test/SecretLockInfoCacheTestUtils.h"

namespace catapult { namespace mongo { class MongoStorageContext; } }

namespace catapult { namespace test {

	/// Mongo traits for a secret lock info.
	struct MongoSecretLockInfoTestTraits : public BasicSecretLockInfoTestTraits {
		/// Creates a catapult cache.
		static cache::CatapultCache CreateCatapultCache();

		/// Creates a mongo secret lock info cache storage around \a context.
		static std::unique_ptr<mongo::ExternalCacheStorage> CreateMongoCacheStorage(mongo::MongoStorageContext& context);
	};
}}
