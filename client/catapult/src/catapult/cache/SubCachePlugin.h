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
#include "catapult/plugins.h"
#include "catapult/types.h"
#include <memory>
#include <string>

namespace catapult {
	namespace cache {
		class CacheChangesStorage;
		class CacheStorage;
		class CatapultCache;
	}
}

namespace catapult { namespace cache {

	// region SubCacheViewIdentifier

	/// Sub cache view types.
	enum class SubCacheViewType {
		/// Cache view.
		View,

		/// Cache delta.
		Delta,

		/// Cache detached delta.
		DetachedDelta
	};

	/// Sub cache view identifier.
	struct SubCacheViewIdentifier {
		/// Cache name.
		/// \note std::array instead of std::string to guarantee string inlining in struct.
		std::array<char, 16> CacheName;

		/// Cache id.
		size_t CacheId;

		/// View type.
		SubCacheViewType ViewType;
	};

	// endregion

	// region SubCacheView / DetachedSubCacheView

	/// Sub cache view.
	class PLUGIN_API_DEPENDENCY SubCacheView {
	public:
		virtual ~SubCacheView() = default;

	public:
		/// Gets the view identifier.
		virtual const SubCacheViewIdentifier& id() const = 0;

		/// Gets a const pointer to the underlying view.
		virtual const void* get() const = 0;

		/// Gets a pointer to the underlying view.
		virtual void* get() = 0;

		/// Returns \c true if cache supports merkle root.
		virtual bool supportsMerkleRoot() const = 0;

		/// Gets the cache merkle root (\a merkleRoot) if supported.
		virtual bool tryGetMerkleRoot(Hash256& merkleRoot) const = 0;

		/// Sets the cache merkle root (\a merkleRoot) if supported.
		virtual bool trySetMerkleRoot(const Hash256& merkleRoot) = 0;

		/// Recalculates the merkle root given the specified chain \a height if supported.
		virtual void updateMerkleRoot(Height height) = 0;

		/// Prunes the cache at \a height.
		virtual void prune(Height height) = 0;

		/// Gets a read-only view of this view.
		virtual const void* asReadOnly() const = 0;
	};

	/// Detached sub cache view.
	class PLUGIN_API_DEPENDENCY DetachedSubCacheView {
	public:
		virtual ~DetachedSubCacheView() = default;

	public:
		/// Locks the cache delta.
		/// \note Returns \c nullptr if the detached delta is no longer valid.
		virtual std::unique_ptr<SubCacheView> tryLock() = 0;
	};

	// endregion

	// region SubCachePlugin

	/// Sub cache plugin that can be added to the main catapult cache.
	class SubCachePlugin {
	public:
		virtual ~SubCachePlugin() = default;

	public:
		/// Gets the cache name.
		virtual const std::string& name() const = 0;

		/// Gets the cache id.
		virtual size_t id() const = 0;

	public:
		/// Gets a locked cache view based on this cache.
		virtual std::unique_ptr<const SubCacheView> createView() const = 0;

		/// Gets a locked cache delta based on this cache.
		/// \note Changes to an attached delta can be committed by calling commit.
		virtual std::unique_ptr<SubCacheView> createDelta() = 0;

		/// Gets a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		virtual std::unique_ptr<DetachedSubCacheView> createDetachedDelta() const = 0;

		/// Commits all pending changes to the underlying storage.
		virtual void commit() = 0;

	public:
		/// Gets a const pointer to the underlying cache.
		virtual const void* get() const = 0;

	public:
		/// Gets a cache storage based on this cache.
		virtual std::unique_ptr<CacheStorage> createStorage() = 0;

		/// Gets a cache changes storage based on this cache.
		virtual std::unique_ptr<CacheChangesStorage> createChangesStorage() const = 0;
	};

	// endregion
}}
