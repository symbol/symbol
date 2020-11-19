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
#include "SecretLockInfoBaseSets.h"
#include "SecretLockInfoCacheSerializers.h"
#include "plugins/txes/lock_shared/src/cache/LockInfoCacheDelta.h"

namespace catapult { namespace cache {

	/// Basic delta on top of the secret lock info cache.
	class BasicSecretLockInfoCacheDelta : public BasicLockInfoCacheDelta<SecretLockInfoCacheDescriptor, SecretLockInfoCacheTypes> {
	public:
		using BasicLockInfoCacheDelta<SecretLockInfoCacheDescriptor, SecretLockInfoCacheTypes>::BasicLockInfoCacheDelta;
	};

	/// Delta on top of the secret lock info cache.
	class SecretLockInfoCacheDelta
			: public LockInfoCacheDelta<SecretLockInfoCacheDescriptor, SecretLockInfoCacheTypes, BasicSecretLockInfoCacheDelta> {
	public:
		using LockInfoCacheDelta<
				SecretLockInfoCacheDescriptor,
				SecretLockInfoCacheTypes,
				BasicSecretLockInfoCacheDelta>::LockInfoCacheDelta;
	};
}}
