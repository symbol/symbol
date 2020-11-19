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

#include "src/handlers/ConfirmedTimestampedHashesProducerFactory.h"
#include "src/cache/HashCache.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS ConfirmedTimestampedHashesProducerFactoryTests

	namespace {
		using TimestampedHashes = std::vector<state::TimestampedHash>;
		using TimestampedHashPointers = std::vector<const state::TimestampedHash*>;
		using state::TimestampedHashRange;

		auto ToTimestampedHashRange(const TimestampedHashes& timestampedHashes, const std::vector<size_t>& indexes) {
			TimestampedHashes temp;
			for (auto index : indexes)
				temp.push_back(timestampedHashes[index]);

			return state::TimestampedHashRange::CopyFixed(reinterpret_cast<const uint8_t*>(temp.data()), indexes.size());
		}

		auto CreateTimestampedHashes(size_t count) {
			TimestampedHashes timestampedHashes;
			for (auto i = 0u; i < count; ++i)
				timestampedHashes.emplace_back(Timestamp(5 * i), Hash256{ { static_cast<uint8_t>(i + 1) } });

			return timestampedHashes;
		}

		auto CreateCacheWithAlternatingHashes(const TimestampedHashes& timestampedHashes) {
			// only insert every second timestamped hash into the cache
			auto pCache = std::make_unique<cache::HashCache>(cache::CacheConfiguration(), utils::TimeSpan::FromHours(1));
			auto delta = pCache->createDelta();
			auto i = 0u;
			for (auto timestampedHash : timestampedHashes)
				if (0 == i++ % 2)
					delta->insert(timestampedHash);

			pCache->commit();
			return pCache;
		}

		void AssertEqual(const TimestampedHashRange& expectedEntities, const TimestampedHashPointers& actualEntities) {
			ASSERT_EQ(expectedEntities.size(), actualEntities.size());

			auto i = 0u;
			for (const auto& timestampedHash : expectedEntities) {
				EXPECT_EQ(timestampedHash, *actualEntities[i]) << "timestamped hash at " << i;
				++i;
			}
		}

		struct ConfirmedTimestampedHashesContext {
			explicit ConfirmedTimestampedHashesContext(size_t count)
					: TimestampedHashes(CreateTimestampedHashes(count))
					, pCache(CreateCacheWithAlternatingHashes(TimestampedHashes))
					, ProducerFactory(CreateConfirmedTimestampedHashesProducerFactory(*pCache))
			{}

			handlers::TimestampedHashes TimestampedHashes;
			std::unique_ptr<cache::HashCache> pCache;
			handlers::ConfirmedTimestampedHashesProducerFactory ProducerFactory;
		};

		constexpr size_t Num_Timestamped_Hashes = 10;

		void AssertCanFilterTimestampedHashes(const std::vector<size_t>& requestIndexes, const std::vector<size_t>& responseIndexes) {
			// Arrange:
			ConfirmedTimestampedHashesContext context(Num_Timestamped_Hashes);
			auto requestRange = ToTimestampedHashRange(context.TimestampedHashes, requestIndexes);
			auto expectedResponse = ToTimestampedHashRange(context.TimestampedHashes, responseIndexes);

			// Act:
			auto unknownTimestampedHashesProducer = context.ProducerFactory(requestRange);

			// Assert:
			AssertEqual(expectedResponse, test::ProduceAll(unknownTimestampedHashesProducer));
		}
	}

	TEST(TEST_CLASS, CanFilterSingleKnownTimestampedHash) {
		for (auto i = 0u; i < Num_Timestamped_Hashes; i += 2)
			AssertCanFilterTimestampedHashes({ i }, {});
	}

	TEST(TEST_CLASS, SingleUnknownTimestampedHashDoesNotGetFiltered) {
		for (auto i = 1u; i < Num_Timestamped_Hashes; i += 2)
			AssertCanFilterTimestampedHashes({ i }, { i });
	}

	TEST(TEST_CLASS, CanFilterMultipleKnownTimestampedHashes) {
		AssertCanFilterTimestampedHashes({ 0, 4, 8 }, {});
		AssertCanFilterTimestampedHashes({ 0, 8 }, {});
	}

	TEST(TEST_CLASS, MultipleUnknownTimestampedHashesDoNotGetFiltered) {
		AssertCanFilterTimestampedHashes({ 1, 5, 9 }, { 1, 5, 9 });
		AssertCanFilterTimestampedHashes({ 5, 9 }, { 5, 9 });
	}

	TEST(TEST_CLASS, OnlyKnownTimestampedHashesGetFiltered) {
		AssertCanFilterTimestampedHashes({ 1, 2, 4 }, { 1 });
		AssertCanFilterTimestampedHashes({ 2, 4, 5, 8 }, { 5 });
		AssertCanFilterTimestampedHashes({ 4, 5, 7 }, { 5, 7 });
		AssertCanFilterTimestampedHashes({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, { 1, 3, 5, 7, 9 });
	}
}}
