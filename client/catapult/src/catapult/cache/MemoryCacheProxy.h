#pragma once
#include "MemoryCacheOptions.h"
#include <memory>

namespace catapult { namespace cache {

	/// A delegating proxy around a memory-based cache.
	template<typename TMemoryCache, typename TCache, typename TCacheModifierProxy>
	class MemoryCacheProxy : public TCache {
	public:
		/// Creates a proxy around \a options.
		explicit MemoryCacheProxy(const MemoryCacheOptions& options)
				: m_memoryCache(options)
				, m_pCache(&m_memoryCache)
		{}

		/// Creates a proxy around \a options and the write-only cache created by
		/// \a factory with \a args arguments.
		template<typename TMutableCacheFactory, typename... TArgs>
		explicit MemoryCacheProxy(const MemoryCacheOptions& options, TMutableCacheFactory factory, TArgs&&... args)
				: m_memoryCache(options)
				, m_pMutableCache(factory(m_memoryCache, std::forward<TArgs>(args)...))
				, m_pCache(m_pMutableCache.get())
		{}

	public:
		/// Gets a read only view based on this cache.
		auto view() const {
			return m_memoryCache.view();
		}

		/// Implicitly casts this proxy to a (const) memory cache.
		operator const TMemoryCache&() const {
			return m_memoryCache;
		}

	public:
		TCacheModifierProxy modifier() override {
			return m_pCache->modifier();
		}

	private:
		TMemoryCache m_memoryCache;
		std::unique_ptr<TCache> m_pMutableCache;
		TCache* m_pCache;
	};
}}
