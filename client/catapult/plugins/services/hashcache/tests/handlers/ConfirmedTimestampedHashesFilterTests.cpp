#include "src/handlers/ConfirmedTimestampedHashesFilter.h"
#include "src/cache/HashCache.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

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
			auto pCache = std::make_unique<cache::HashCache>(utils::TimeSpan::FromHours(1));
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
					, Filter(CreateConfirmedTimestampedHashesFilter(*pCache))
			{}

			handlers::TimestampedHashes TimestampedHashes;
			std::unique_ptr<cache::HashCache> pCache;
			handlers::ConfirmedTimestampedHashesFilter Filter;
		};
	}

	namespace {
		constexpr size_t Num_Timestamped_Hashes = 10;

		void AssertCanFilterTimestampedHashes(const std::vector<size_t>& requestIndexes, const std::vector<size_t>& responseIndexes) {
			// Arrange:
			ConfirmedTimestampedHashesContext context(Num_Timestamped_Hashes);
			auto requestRange = ToTimestampedHashRange(context.TimestampedHashes, requestIndexes);
			auto expectedResponse = ToTimestampedHashRange(context.TimestampedHashes, responseIndexes);

			// Act:
			auto unknownTimestampedHashes = context.Filter(requestRange);

			// Assert:
			AssertEqual(expectedResponse, unknownTimestampedHashes);
		}
	}

	TEST(ConfirmedTimestampedHashesFilterTests, CanFilterSingleKnownTimestampedHash) {
		// Assert:
		for (auto i = 0u; i < Num_Timestamped_Hashes; i += 2)
			AssertCanFilterTimestampedHashes({ i }, {});
	}

	TEST(ConfirmedTimestampedHashesFilterTests, SingleUnknownTimestampedHashDoesNotGetFiltered) {
		// Assert:
		for (auto i = 1u; i < Num_Timestamped_Hashes; i += 2)
			AssertCanFilterTimestampedHashes({ i }, { i });
	}

	TEST(ConfirmedTimestampedHashesFilterTests, CanFilterMultipleKnownTimestampedHashes) {
		// Assert:
		AssertCanFilterTimestampedHashes({ 0, 4, 8 }, {});
		AssertCanFilterTimestampedHashes({ 0, 8 }, {});
	}

	TEST(ConfirmedTimestampedHashesFilterTests, MultipleUnknownTimestampedHashesDoNotGetFiltered) {
		// Assert:
		AssertCanFilterTimestampedHashes({ 1, 5, 9 }, { 1, 5, 9 });
		AssertCanFilterTimestampedHashes({ 5, 9 }, { 5, 9 });
	}

	TEST(ConfirmedTimestampedHashesFilterTests, OnlyKnownTimestampedHashesGetFiltered) {
		// Assert:
		AssertCanFilterTimestampedHashes({ 1, 2, 4 }, { 1 });
		AssertCanFilterTimestampedHashes({ 2, 4, 5, 8 }, { 5 });
		AssertCanFilterTimestampedHashes({ 4, 5, 7 }, { 5, 7 });
		AssertCanFilterTimestampedHashes({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, { 1, 3, 5, 7, 9 });
	}
}}
