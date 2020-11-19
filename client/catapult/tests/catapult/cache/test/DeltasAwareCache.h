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
#include "ByteVectorCacheChanges.h"
#include "catapult/cache/SynchronizedCache.h"
#include "catapult/deltaset/DeltaElementsMixin.h"
#include "catapult/exceptions.h"
#include "tests/test/other/DeltaElementsTestUtils.h"

namespace catapult { namespace test {

	/// Emulates a deltaset that exposes deltas().
	using ByteVectorCacheDeltas = test::DeltaElementsTestUtils::Wrapper<std::map<uint8_t, std::vector<uint8_t>>>;

	// emulate a basic cache that supports deltas reporting
	class BasicDeltasAwareCache {
	public:
		enum class OperationType { Load_Into, Purge };

		using CacheValueType = std::vector<uint8_t>;
		using Breadcrumbs = std::vector<std::pair<std::vector<uint8_t>, std::vector<OperationType>>>;

		// stub out view-required size, tryMakeIterableView and asReadOnly functions
		struct CacheViewType {
			size_t size() const {
				return 0;
			}

			std::vector<std::vector<uint8_t>>* tryMakeIterableView() const {
				return nullptr;
			}

			const CacheViewType& asReadOnly() const {
				return *this;
			}
		};

		// stub out view-required functions and expose deltas
		struct CacheDeltaType : public CacheViewType, public deltaset::DeltaElementsMixin<ByteVectorCacheDeltas> {
		public:
			CacheDeltaType(const ByteVectorCacheDeltas& deltas, BasicDeltasAwareCache::Breadcrumbs& breadcrumbs)
					: deltaset::DeltaElementsMixin<ByteVectorCacheDeltas>::DeltaElementsMixin(deltas)
					, m_breadcrumbs(breadcrumbs)
			{}

		public:
			void push(const std::vector<uint8_t>& value, OperationType operationType) {
				if (m_breadcrumbs.empty() || m_breadcrumbs.back().first != value)
					m_breadcrumbs.emplace_back(value, std::vector<OperationType>());

				m_breadcrumbs.back().second.push_back(operationType);
			}

		private:
			BasicDeltasAwareCache::Breadcrumbs& m_breadcrumbs;
		};

		using CacheReadOnlyType = CacheViewType;

	public:
		BasicDeltasAwareCache(const ByteVectorCacheDeltas& deltas, Breadcrumbs& breadcrumbs)
				: m_deltas(deltas)
				, m_breadcrumbs(breadcrumbs)
		{}

	public:
		CacheViewType createView() const {
			CATAPULT_THROW_RUNTIME_ERROR("createView - not supported");
		}

		CacheDeltaType createDelta() {
			return CacheDeltaType(m_deltas, m_breadcrumbs);
		}

		CacheDeltaType createDetachedDelta() const {
			CATAPULT_THROW_RUNTIME_ERROR("createDetachedDelta - not supported");
		}

		void commit(const CacheDeltaType&)
		{}

	private:
		const ByteVectorCacheDeltas& m_deltas;
		Breadcrumbs& m_breadcrumbs;
	};

	// wrap BasicDeltasAwareCache in cache::SynchronizedCache so it can be registered with CatapultCache
	template<size_t CacheId>
	class DeltasAwareCache : public cache::SynchronizedCache<BasicDeltasAwareCache> {
	public:
		static constexpr auto Id = CacheId;
		static constexpr auto Name = "DeltasAwareCache";

	public:
		explicit DeltasAwareCache(const ByteVectorCacheDeltas& deltas) : DeltasAwareCache(deltas, m_breadcrumbs)
		{}

		DeltasAwareCache(const ByteVectorCacheDeltas& deltas, BasicDeltasAwareCache::Breadcrumbs& breadcrumbs)
				: SynchronizedCache(BasicDeltasAwareCache(deltas, breadcrumbs))
		{}

	private:
		BasicDeltasAwareCache::Breadcrumbs m_breadcrumbs; // default breadcrumb backing when not supplied to constructor
	};

	// stub out storage traits to allow DeltasAwareCache to be added to cache via builder
	struct DeltasAwareCacheStorageTraits : public ByteVectorSerializer {
		using DestinationType = BasicDeltasAwareCache::CacheDeltaType;

		static void LoadInto(const std::vector<uint8_t>& value, DestinationType& cacheDelta) {
			cacheDelta.push(value, BasicDeltasAwareCache::OperationType::Load_Into);
		}

		static void Purge(const std::vector<uint8_t>& value, DestinationType& cacheDelta) {
			cacheDelta.push(value, BasicDeltasAwareCache::OperationType::Purge);
		}
	};
}}
