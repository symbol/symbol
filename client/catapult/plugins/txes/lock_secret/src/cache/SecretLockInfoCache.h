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
#include "SecretLockInfoCacheDelta.h"
#include "SecretLockInfoCacheStorage.h"
#include "SecretLockInfoCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	/// Cache composed of secret lock info information.
	using BasicSecretLockInfoCache = BasicCache<SecretLockInfoCacheDescriptor, SecretLockInfoCacheTypes::BaseSets>;

	/// Synchronized cache composed of secret lock info information.
	class SecretLockInfoCache : public SynchronizedCache<BasicSecretLockInfoCache> {
	public:
		DEFINE_CACHE_CONSTANTS(SecretLockInfo)

	public:
		/// Creates a cache around \a config.
		explicit SecretLockInfoCache(const CacheConfiguration& config)
				: SynchronizedCache<BasicSecretLockInfoCache>(BasicSecretLockInfoCache(config))
		{}
	};
}}
