#include "catapult/local/LocalNodeStats.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

	namespace {
		class StatsSource {
		public:
			size_t value() {
				return 7;
			}
		};
	}

	TEST(LocalNodeStatsTests, GetStatsValueReturnsSentinelValueWhenSourceIsNull) {
		// Arrange:
		auto pSource = std::shared_ptr<StatsSource>();

		// Act:
		auto value = GetStatsValue(pSource, &StatsSource::value);

		// Assert:
		EXPECT_EQ(Sentinel_Stats_Value, value);
	}

	TEST(LocalNodeStatsTests, GetStatsValueReturnsValueWhenSourceIsNotNull) {
		// Arrange:
		auto pSource = std::make_shared<StatsSource>();

		// Act:
		auto value = GetStatsValue(pSource, &StatsSource::value);

		// Assert:
		EXPECT_EQ(7u, value);
	}
}}
