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
#include <memory>
#include <string>

namespace catapult {
	namespace cache {
		class CacheStorage;
		class CatapultCache;
	}
}

namespace catapult { namespace cache {

	/// A subcache view.
	class SubCacheView {
	public:
		virtual ~SubCacheView() {}

	public:
		/// Returns a const pointer to the underlying view.
		virtual const void* get() const = 0;

		/// Returns a pointer to the underlying view.
		virtual void* get() = 0;

		/// Returns a read-only view of this view.
		virtual const void* asReadOnly() const = 0;
	};

	/// A detached subcache view.
	class DetachedSubCacheView {
	public:
		virtual ~DetachedSubCacheView() {}

	public:
		/// Locks the cache delta.
		/// \note Returns \c nullptr if the detached delta is no longer valid.
		virtual std::unique_ptr<SubCacheView> lock() = 0;
	};

	/// A subcache plugin that can be added to the main catapult cache.
	class SubCachePlugin {
	public:
		virtual ~SubCachePlugin() {}

	public:
		/// Gets the cache name.
		virtual const std::string& name() const = 0;

	public:
		/// Returns a locked cache view based on this cache.
		virtual std::unique_ptr<const SubCacheView> createView() const = 0;

		/// Returns a locked cache delta based on this cache.
		/// \note Changes to an attached delta can be committed by calling commit.
		virtual std::unique_ptr<SubCacheView> createDelta() = 0;

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		virtual std::unique_ptr<DetachedSubCacheView> createDetachedDelta() const = 0;

		/// Commits all pending changes to the underlying storage.
		virtual void commit() = 0;

	public:
		/// Returns a const pointer to the underlying cache.
		virtual const void* get() const = 0;

	public:
		/// Returns a cache storage based on this cache.
		virtual std::unique_ptr<CacheStorage> createStorage() = 0;
	};
}}
