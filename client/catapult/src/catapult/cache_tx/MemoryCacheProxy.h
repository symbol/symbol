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
#include "MemoryCacheOptions.h"
#include <memory>

namespace catapult { namespace cache {

	/// Delegating proxy around a memory-based cache.
	/// \note This extra level of indirection is present to support both
	///       (1) dynamic modifier decoration
	///       (2) by value view/modifier semantics for consistency across the codebase.
	template<typename TMemoryCache>
	class MemoryCacheProxy : public TMemoryCache::CacheWriteOnlyInterface {
	private:
		using CacheModifierProxy = typename TMemoryCache::CacheModifierProxy;
		using CacheWriteOnlyInterface = typename TMemoryCache::CacheWriteOnlyInterface;
		using CacheReadWriteInterface = typename TMemoryCache::CacheReadWriteInterface;

	public:
		/// Creates a proxy around \a options.
		explicit MemoryCacheProxy(const MemoryCacheOptions& options)
				: m_memoryCache(options)
				, m_pCache(&m_memoryCache)
		{}

		/// Creates a proxy around \a options and the write-only cache created by
		/// \a factory with \a args arguments.
		template<typename TMutableCacheFactory, typename... TArgs>
		MemoryCacheProxy(const MemoryCacheOptions& options, TMutableCacheFactory factory, TArgs&&... args)
				: m_memoryCache(options)
				, m_pMutableCache(factory(m_memoryCache, std::forward<TArgs>(args)...))
				, m_pCache(m_pMutableCache.get())
		{}

	public:
		/// Gets the underlying (const) read write cache.
		const CacheReadWriteInterface& get() const {
			return m_memoryCache;
		}

		/// Gets the underlying (non-const) write only cache.
		CacheWriteOnlyInterface& get() {
			return *m_pCache;
		}

	public:
		/// Gets a read only view based on this cache.
		/// \note This is non-virtual because MemoryCacheProxy derives from CacheWriteOnlyInterface but not CacheReadWriteInterface.
		///       Aggregate implementations used to trigger subscriptions only need to intercept modifier but not view calls.
		///       The presence of this function allows for nicer usage when using proxy directly.
		auto view() const {
			return m_memoryCache.view();
		}

	public:
		CacheModifierProxy modifier() override {
			return m_pCache->modifier();
		}

	private:
		TMemoryCache m_memoryCache;
		std::unique_ptr<CacheWriteOnlyInterface> m_pMutableCache;
		CacheWriteOnlyInterface* m_pCache;
	};
}}
