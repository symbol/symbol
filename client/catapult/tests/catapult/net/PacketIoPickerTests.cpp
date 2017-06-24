#include "catapult/net/PacketIoPicker.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/test/core/mocks/MockPacketIoPicker.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

	namespace {
		const auto Default_Timeout = []() { return utils::TimeSpan::FromMinutes(1); }();

		void AssertPickMultiple(size_t numAvailable, size_t numRequested, size_t expectedNumReturned, size_t expectedNumPicks) {
			// Arrange:
			mocks::MockPacketIoPicker picker(numAvailable);

			// Act:
			auto packetIos = PickMultiple(picker, numRequested, Default_Timeout);

			// Assert:
			EXPECT_EQ(expectedNumReturned, packetIos.size());
			for (auto i = 0u; i < packetIos.size(); ++i)
				EXPECT_EQ(std::to_string(i + 1), packetIos[i].node().Identity.Name) << "packetIo at " << i;

			auto i = 0u;
			EXPECT_EQ(expectedNumPicks, picker.durations().size());
			for (const auto& duration : picker.durations())
				EXPECT_EQ(Default_Timeout, duration) << "duration at " << i++;
		}
	}

	TEST(PacketIoPickerTests, PickMultipleReturnsZeroIosIfZeroAreRequested) {
		// Assert:
		AssertPickMultiple(5, 0, 0, 0);
	}

	TEST(PacketIoPickerTests, PickMultipleReturnsZeroIosIfZeroAreAvailable) {
		// Assert:
		AssertPickMultiple(0, 5, 0, 1);
	}

	TEST(PacketIoPickerTests, PickMultipleReturnsAllIosIfRequestedIsGreaterThanAvailable) {
		// Assert:
		AssertPickMultiple(3, 5, 3, 4);
	}

	TEST(PacketIoPickerTests, PickMultipleReturnsRequestedIosIfActualIsAtLeastRequested) {
		// Assert:
		AssertPickMultiple(5, 5, 5, 5);
		AssertPickMultiple(9, 5, 5, 5);
	}
}}
