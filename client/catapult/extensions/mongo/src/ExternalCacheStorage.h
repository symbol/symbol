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
#include "catapult/cache/CacheChanges.h"
#include "catapult/functions.h"

namespace catapult { namespace mongo {

	/// Abstract class for saving cache data to external storage.
	class ExternalCacheStorage {
	protected:
		/// Creates an external cache storage around \a name and \a id.
		ExternalCacheStorage(const std::string& name, size_t id)
				: m_name(name)
				, m_id(id)
		{}

	public:
		virtual ~ExternalCacheStorage() = default;

	public:
		/// Gets the cache name.
		const std::string& name() const {
			return m_name;
		}

		/// Gets the cache id.
		size_t id() const {
			return m_id;
		}

	public:
		/// Saves cache \a changes to external storage.
		virtual void saveDelta(const cache::CacheChanges& changes) = 0;

	private:
		std::string m_name;
		size_t m_id;
	};

	/// Typed interface for saving cache data to external storage.
	template<typename TCache>
	class ExternalCacheStorageT : public ExternalCacheStorage {
	public:
		/// Creates an external cache storage.
		ExternalCacheStorageT() : ExternalCacheStorage(TCache::Name, TCache::Id)
		{}

	public:
		void saveDelta(const cache::CacheChanges& changes) final override {
			saveDelta(changes.sub<TCache>());
		}

	private:
		using CacheChangesType = cache::SingleCacheChangesT<typename TCache::CacheDeltaType, typename TCache::CacheValueType>;

		/// Saves cache \a changes to external storage.
		virtual void saveDelta(const CacheChangesType& changes) = 0;
	};
}}
