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
#include "CatapultCacheDelta.h"
#include <unordered_set>
#include <vector>

namespace catapult { namespace cache {

	// region MemoryCacheChanges

	/// Deserialized cache changes for a single cache.
	/// \note This is used for tagging.
	struct PLUGIN_API_DEPENDENCY MemoryCacheChanges : public utils::NonCopyable {
		virtual ~MemoryCacheChanges() = default;
	};

	/// Deserialized cache changes for a single cache.
	template<typename TValue>
	struct PLUGIN_API_DEPENDENCY MemoryCacheChangesT : public MemoryCacheChanges {
		/// Added elements.
		std::vector<TValue> Added;

		/// Removed elements.
		std::vector<TValue> Removed;

		/// Copied elements.
		std::vector<TValue> Copied;
	};

	// endregion

	// region SingleCacheChanges

	/// Provides common view of single sub cache changes.
	/// \note This is used for tagging.
	struct SingleCacheChanges : public utils::NonCopyable {};

	/// Provides common view of single sub cache changes.
	template<typename TCacheDelta, typename TValue>
	class SingleCacheChangesT : public SingleCacheChanges {
	private:
		using PointerContainer = decltype(reinterpret_cast<const TCacheDelta*>(0)->addedElements());

	public:
		/// Creates changes around \a cacheDelta.
		explicit SingleCacheChangesT(const TCacheDelta& cacheDelta)
				: m_pCacheDelta(&cacheDelta)
				, m_pMemoryCacheChanges(nullptr)
		{}

		/// Creates changes around \a memoryCacheChanges.
		explicit SingleCacheChangesT(const MemoryCacheChangesT<TValue>& memoryCacheChanges)
				: m_pCacheDelta(nullptr)
				, m_pMemoryCacheChanges(&memoryCacheChanges)
		{}

	public:
		/// Gets the pointers to all added elements.
		PointerContainer addedElements() const {
			return m_pCacheDelta ? m_pCacheDelta->addedElements() : CollectAllPointers(m_pMemoryCacheChanges->Added);
		}

		/// Gets the pointers to all modified elements.
		PointerContainer modifiedElements() const {
			return m_pCacheDelta ? m_pCacheDelta->modifiedElements() : CollectAllPointers(m_pMemoryCacheChanges->Copied);
		}

		/// Gets the pointers to all removed elements.
		PointerContainer removedElements() const {
			return m_pCacheDelta ? m_pCacheDelta->removedElements() : CollectAllPointers(m_pMemoryCacheChanges->Removed);
		}

	private:
		static PointerContainer CollectAllPointers(const std::vector<TValue>& values) {
			PointerContainer pointers;
			for (const auto& value : values)
				pointers.insert(&value);

			return pointers;
		}

	private:
		const TCacheDelta* m_pCacheDelta;
		const MemoryCacheChangesT<TValue>* m_pMemoryCacheChanges;
	};

	// endregion

	// region CacheChanges

	/// Provides common view of aggregate cache changes.
	class CacheChanges : public utils::MoveOnly {
	public:
		/// Memory cache changes container.
		using MemoryCacheChangesContainer = std::vector<std::unique_ptr<const MemoryCacheChanges>>;

	public:
		/// Creates changes around \a cacheDelta.
		explicit CacheChanges(const CatapultCacheDelta& cacheDelta) : m_pCacheDelta(&cacheDelta)
		{}

		/// Creates changes around \a memoryCacheChangesContainer.
		explicit CacheChanges(MemoryCacheChangesContainer&& memoryCacheChangesContainer)
				: m_pCacheDelta(nullptr)
				, m_memoryCacheChangesContainer(std::move(memoryCacheChangesContainer))
		{}

	public:
		/// Gets a specific sub cache changes view.
		template<typename TCache>
		auto sub() const {
			using SubCacheChanges = SingleCacheChangesT<typename TCache::CacheDeltaType, typename TCache::CacheValueType>;
			if (m_pCacheDelta)
				return SubCacheChanges(m_pCacheDelta->sub<TCache>());

			using TypedMemoryCacheChanges = MemoryCacheChangesT<typename TCache::CacheValueType>;
			return SubCacheChanges(*static_cast<const TypedMemoryCacheChanges*>(m_memoryCacheChangesContainer[TCache::Id].get()));
		}

	private:
		const CatapultCacheDelta* m_pCacheDelta;
		MemoryCacheChangesContainer m_memoryCacheChangesContainer;
	};

	// endregion
}}
