#include "catapult/net/ConnectionSettings.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

	TEST(ConnectionSettingsTests, CanCreateDefaultConnectionSettings) {
		// Act:
		auto settings = ConnectionSettings();

		// Assert:
		EXPECT_EQ(model::NetworkIdentifier::Zero, settings.NetworkIdentifier);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(10), settings.Timeout);
		EXPECT_EQ(utils::FileSize::FromKilobytes(4), settings.SocketWorkingBufferSize);
		EXPECT_EQ(utils::FileSize::FromMegabytes(100), settings.MaxPacketDataSize);
	}

	TEST(ConnectionSettingsTests, CanConvertToPacketSocketOptions) {
		// Arrange:
		auto settings = ConnectionSettings();
		settings.SocketWorkingBufferSize = utils::FileSize::FromKilobytes(54);
		settings.MaxPacketDataSize = utils::FileSize::FromMegabytes(2);

		// Act:
		auto options = settings.toSocketOptions();

		// Assert:
		EXPECT_EQ(54u * 1024, options.WorkingBufferSize);
		EXPECT_EQ(2u * 1024 * 1024, options.MaxPacketDataSize);
	}
}}
