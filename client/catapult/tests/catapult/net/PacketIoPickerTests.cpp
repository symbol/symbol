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

#include "catapult/net/PacketIoPicker.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/test/core/mocks/MockPacketIoPicker.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

#define TEST_CLASS PacketIoPickerTests

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
				EXPECT_EQ(std::to_string(i + 1), packetIos[i].node().metadata().Name) << "packetIo at " << i;

			auto i = 0u;
			EXPECT_EQ(expectedNumPicks, picker.durations().size());
			for (const auto& duration : picker.durations())
				EXPECT_EQ(Default_Timeout, duration) << "duration at " << i++;
		}
	}

	TEST(TEST_CLASS, PickMultipleReturnsZeroIosWhenZeroAreRequested) {
		AssertPickMultiple(5, 0, 0, 0);
	}

	TEST(TEST_CLASS, PickMultipleReturnsZeroIosWhenZeroAreAvailable) {
		AssertPickMultiple(0, 5, 0, 1);
	}

	TEST(TEST_CLASS, PickMultipleReturnsAllIosWhenRequestedIsGreaterThanAvailable) {
		AssertPickMultiple(3, 5, 3, 4);
	}

	TEST(TEST_CLASS, PickMultipleReturnsRequestedIosWhenActualIsAtLeastRequested) {
		AssertPickMultiple(5, 5, 5, 5);
		AssertPickMultiple(9, 5, 5, 5);
	}
}}
