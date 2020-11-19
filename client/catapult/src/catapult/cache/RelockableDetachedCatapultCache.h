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
#include "CatapultCacheDelta.h"
#include "catapult/types.h"

namespace catapult { namespace cache { class CatapultCache; } }

namespace catapult { namespace cache {

	/// Relockable detached catapult cache.
	class RelockableDetachedCatapultCache {
	public:
		/// Creates a relockable detached catapult cache around \a catapultCache.
		explicit RelockableDetachedCatapultCache(const CatapultCache& catapultCache);

		/// Destroys the relockable detached catapult cache.
		~RelockableDetachedCatapultCache();

	public:
		/// Gets the current cache height.
		Height height() const;

		/// Gets the last (detached) catapult cache delta and locks it.
		/// \note If locking fails, \c nullptr is returned.
		std::unique_ptr<CatapultCacheDelta> getAndTryLock();

		/// Rebases and locks the (detached) catapult cache delta.
		std::unique_ptr<CatapultCacheDelta> rebaseAndLock();

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}}
