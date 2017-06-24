#pragma once
#include "tests/TestHarness.h"
#include <vector>

namespace catapult { namespace test {

	/// Basic traits for a set-based cache.
	template<typename TCache, typename TValue, typename TEntityTraits>
	struct SetCacheTraits {
	public:
		using EntityTraits = TEntityTraits;
		using EntityVector = std::vector<TValue>;

	public:
		/// Inserts \a value into \a delta.
		static void Insert(typename TCache::CacheDeltaType& delta, const TValue& value) {
			delta.insert(value);
		}

		/// Removes \a value from \a delta.
		static void Remove(typename TCache::CacheDeltaType& delta, const TValue& value) {
			delta.remove(value);
		}

		/// Inserts multiple values with the specified identifiers (\a ids) into \a cache.
		static EntityVector InsertMultiple(TCache& cache, std::initializer_list<size_t> ids) {
			auto delta = cache.createDelta();
			EntityVector entities;
			for (auto id : ids) {
				auto value = EntityTraits::CreateEntity(id);
				delta->insert(value);
				entities.push_back(value);
			}

			cache.commit();
			return entities;
		}
	};

	/// Asserts that \a cache exactly contains entities in \a expectedEntities.
	template<typename TCache, typename TEntityVector>
	void AssertCacheContents(const TCache& cache, const TEntityVector& expectedEntities) {
		// Assert:
		EXPECT_EQ(expectedEntities.size(), cache.size());

		auto i = 0u;
		for (const auto& entity : expectedEntities) {
			EXPECT_TRUE(cache.contains(entity)) << "entity at " << i;
			++i;
		}
	}
}}
