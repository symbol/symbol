#include "catapult/net/ConnectionSettings.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

#define TEST_CLASS ConnectionSettingsTests

	TEST(TEST_CLASS, CanCreateDefaultConnectionSettings) {
		// Act:
		auto settings = ConnectionSettings();

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Zero, settings.NetworkIdentifier);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(10), settings.Timeout);
		EXPECT_EQ(utils::FileSize::FromKilobytes(4), settings.SocketWorkingBufferSize);
		EXPECT_EQ(0u, settings.SocketWorkingBufferSensitivity);
		EXPECT_EQ(utils::FileSize::FromMegabytes(100), settings.MaxPacketDataSize);
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
