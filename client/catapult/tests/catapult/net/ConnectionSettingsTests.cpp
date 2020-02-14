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

#include "catapult/net/ConnectionSettings.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

#define TEST_CLASS ConnectionSettingsTests

	TEST(TEST_CLASS, CanCreateDefaultConnectionSettings) {
		// Act:
		auto settings = ConnectionSettings();

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Zero, settings.NetworkIdentifier);
		EXPECT_EQ(model::NodeIdentityEqualityStrategy::Key, settings.NodeIdentityEqualityStrategy);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(10), settings.Timeout);
		EXPECT_EQ(utils::FileSize::FromKilobytes(4), settings.SocketWorkingBufferSize);
		EXPECT_EQ(0u, settings.SocketWorkingBufferSensitivity);
		EXPECT_EQ(utils::FileSize::FromMegabytes(100), settings.MaxPacketDataSize);

		EXPECT_TRUE(settings.AllowIncomingSelfConnections);
		EXPECT_FALSE(settings.AllowOutgoingSelfConnections);
	}

	TEST(TEST_CLASS, CanConvertToPacketSocketOptions) {
		// Arrange:
		auto settings = ConnectionSettings();
		settings.SocketWorkingBufferSize = utils::FileSize::FromKilobytes(54);
		settings.SocketWorkingBufferSensitivity = 123;
		settings.MaxPacketDataSize = utils::FileSize::FromMegabytes(2);

		// Act:
		auto options = settings.toSocketOptions();

		// Assert:
		EXPECT_EQ(54u * 1024, options.WorkingBufferSize);
		EXPECT_EQ(123u, options.WorkingBufferSensitivity);
		EXPECT_EQ(2u * 1024 * 1024, options.MaxPacketDataSize);
	}
}}
