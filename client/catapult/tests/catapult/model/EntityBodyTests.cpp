#include "catapult/model/EntityBody.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS EntityBodyTests

	// region MakeVersion

	TEST(TEST_CLASS, CanMakeVersionFromNetworkIdentifierAndEntityVersion) {
		// Assert:
		EXPECT_EQ(0x0000u, MakeVersion(NetworkIdentifier::Zero, 0u)); // zero version
		EXPECT_EQ(0x9002u, MakeVersion(NetworkIdentifier::Mijin_Test, 2u)); // non zero version
		EXPECT_EQ(0x6802u, MakeVersion(NetworkIdentifier::Public, 2u)); // vary network
		EXPECT_EQ(0x9054u, MakeVersion(NetworkIdentifier::Mijin_Test, 0x54u)); // vary version

		EXPECT_EQ(0xFF54u, MakeVersion(static_cast<NetworkIdentifier>(0xFF), 0x54u)); // max network
		EXPECT_EQ(0x90FFu, MakeVersion(NetworkIdentifier::Mijin_Test, 0xFFu)); // max version
	}

	namespace {
		struct EmptyHeader
		{};

		void AssertNetwork(uint16_t version, uint8_t expectedNetwork) {
			// Act:
			EntityBody<EmptyHeader> entity;
			entity.Version = version;

			// Assert:
			EXPECT_EQ(static_cast<NetworkIdentifier>(expectedNetwork), entity.Network());
		}

		void AssertEntityVersion(uint16_t version, uint8_t expectedEntityVersion) {
			// Act:
			EntityBody<EmptyHeader> entity;
			entity.Version = version;

			// Assert:
			EXPECT_EQ(expectedEntityVersion, entity.EntityVersion());
		}
	}

	TEST(TEST_CLASS, NetworkReturnsNetworkPartOfVersion) {
		// Assert:
		AssertNetwork(0x0000, 0x00);
		AssertNetwork(0x9002, 0x90);
		AssertNetwork(0xFF54, 0xFF);
	}

	TEST(TEST_CLASS, EntityVersionReturnsEntityVersionPartOfVersion) {
		// Assert:
		AssertEntityVersion(0x0000, 0x00);
		AssertEntityVersion(0x9002, 0x02);
		AssertEntityVersion(0x90FF, 0xFF);
	}

	// endregion
}}
