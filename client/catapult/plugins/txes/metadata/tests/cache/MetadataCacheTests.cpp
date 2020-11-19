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

#include "src/cache/MetadataCache.h"
#include "tests/test/MetadataTestUtils.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"

namespace catapult { namespace cache {

#define TEST_CLASS MetadataCacheTests

	// region mixin traits based tests

	namespace {
		struct MetadataCacheMixinTraits {
			class CacheType : public MetadataCache {
			public:
				CacheType() : MetadataCache(CacheConfiguration())
				{}
			};

			using IdType = Hash256;
			using ValueType = state::MetadataEntry;

			static uint8_t GetRawId(const IdType& id) {
				return id[0];
			}

			static IdType GetId(const ValueType& entry) {
				return entry.key().uniqueKey();
			}

			static IdType MakeId(uint8_t id) {
				return IdType{ { id } };
			}

			static ValueType CreateWithId(uint8_t id) {
				return state::MetadataEntry(test::GenerateMetadataKey(MakeId(id)));
			}
		};

		struct MetadataCacheDeltaModificationPolicy : public test:: DeltaInsertModificationPolicy {
			static void Modify(MetadataCacheDelta& delta, const state::MetadataEntry& entry) {
				auto& entryFromCache = delta.find(entry.key().uniqueKey()).get();

				std::vector<uint8_t> valueBuffer{ 0x9A, 0xC7, 0x33 };
				entryFromCache.value().update(valueBuffer);
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(MetadataCacheMixinTraits, ViewAccessor, _View)
	DEFINE_CACHE_CONTAINS_TESTS(MetadataCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_CACHE_ITERATION_TESTS(MetadataCacheMixinTraits, ViewAccessor, _View)

	DEFINE_CACHE_ACCESSOR_TESTS(MetadataCacheMixinTraits, ViewAccessor, MutableAccessor, _ViewMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(MetadataCacheMixinTraits, ViewAccessor, ConstAccessor, _ViewConst)
	DEFINE_CACHE_ACCESSOR_TESTS(MetadataCacheMixinTraits, DeltaAccessor, MutableAccessor, _DeltaMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(MetadataCacheMixinTraits, DeltaAccessor, ConstAccessor, _DeltaConst)

	DEFINE_CACHE_MUTATION_TESTS(MetadataCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(MetadataCacheMixinTraits, MetadataCacheDeltaModificationPolicy, _Delta)

	DEFINE_CACHE_BASIC_TESTS(MetadataCacheMixinTraits,)

	// endregion
}}
