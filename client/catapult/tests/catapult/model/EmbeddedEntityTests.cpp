#include "catapult/model/EmbeddedEntity.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS EmbeddedEntityTests

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(uint32_t) // size
			+ sizeof(uint16_t) // version
			+ sizeof(uint16_t) // entity type
			+ sizeof(Key); // signer

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(EmbeddedEntity));
		EXPECT_EQ(40u, sizeof(EmbeddedEntity));
	}

	// region insertion operator

	TEST(TEST_CLASS, CanOutputEntity) {
		// Arrange:
		EmbeddedEntity entity;
		entity.Size = 121;
		entity.Type = EntityType::Transfer;
		entity.Version = MakeVersion(NetworkIdentifier::Zero, 2);

		// Act:
		auto str = test::ToString(entity);

		// Assert:
		EXPECT_EQ("(embedded) Transfer (v2) with size 121", str);
	}

	// endregion

	// region IsSizeValid

	namespace {
		bool IsSizeValid(const EmbeddedEntity& entity, bool isEmbeddable = true) {
			auto options = isEmbeddable ? mocks::PluginOptionFlags::Default : mocks::PluginOptionFlags::Not_Embeddable;
			auto pRegistry = mocks::CreateDefaultTransactionRegistry(options);
			return IsSizeValid(entity, *pRegistry);
		}
	}

	TEST(TEST_CLASS, SizeIsInvalidForEntityWithUnknownType) {
		// Arrange:
		EmbeddedEntity entity;
		entity.Type = static_cast<EntityType>(-1);
		entity.Size = sizeof(EmbeddedEntity);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(entity));
	}

	namespace {
		std::unique_ptr<EmbeddedEntity> CreateMockEntity(uint32_t delta) {
			auto pEntity = mocks::CreateEmbeddedMockTransaction(7);
			pEntity->Size += delta;
			return std::move(pEntity);
		}
	}

	TEST(TEST_CLASS, SizeIsInvalidForEntityThatDoesNotSupportEmbedding) {
		// Arrange:
		auto pEntity = CreateMockEntity(0);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pEntity, false));
	}

	TEST(TEST_CLASS, SizeIsValidForEntityWithEqualReportedSizeAndActualSize) {
		// Arrange:
		auto pEntity = CreateMockEntity(0);

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pEntity));
	}

	TEST(TEST_CLASS, SizeIsInvalidForEntityWithReportedSizeLessThanActualSize) {
		// Arrange:
		auto pEntity = CreateMockEntity(static_cast<uint32_t>(-1));

		// Act:
		EXPECT_FALSE(IsSizeValid(*pEntity));
	}

	TEST(TEST_CLASS, SizeIsInvalidForEntityWithReportedSizeGreaterThanActualSize) {
		// Arrange:
		auto pEntity = CreateMockEntity(1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pEntity));
	}

	// endregion

	// region PublishNotifications

	TEST(TEST_CLASS, PublishNotificationsPublishesAccountNotifications) {
		// Arrange:
		EmbeddedEntity entity;
		entity.Size = sizeof(EmbeddedEntity);
		test::FillWithRandomData(entity.Signer);
		mocks::MockNotificationSubscriber sub;

		// Act:
		PublishNotifications(entity, sub);

		// Assert:
		EXPECT_EQ(1u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numAddresses());
		EXPECT_EQ(1u, sub.numKeys());

		EXPECT_TRUE(sub.contains(entity.Signer));
	}

	// endregion

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
		void AssertNetwork(uint16_t version, uint8_t expectedNetwork) {
			// Act:
			EmbeddedEntity entity;
			entity.Version = version;

			// Assert:
			EXPECT_EQ(expectedNetwork, entity.Network());
		}

		void AssertEntityVersion(uint16_t version, uint8_t expectedEntityVersion) {
			// Act:
			EmbeddedEntity entity;
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
