#include "catapult/extensions/ServiceUtils.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/extensions/ServerHooks.h"
#include "catapult/extensions/ServiceLocator.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ServiceUtilsTests

	namespace {
		constexpr auto Service_Name = "writers";

		using CosignaturesSink = consumer<const std::vector<model::DetachedCosignature>&>;
		using TransactionSink = extensions::SharedNewTransactionsSink;
	}

	TEST(TEST_CLASS, CanCreatePushEntitySink) {
		// Arrange:
		auto pWriters = std::make_shared<mocks::BroadcastAwareMockPacketWriters>();
		auto keyPair = test::GenerateKeyPair();
		auto transactionInfos = test::CreateTransactionInfos(1);
		auto expectedPayload = ionet::CreateBroadcastPayload(transactionInfos);
		ServiceLocator locator(keyPair);
		locator.registerService(Service_Name, pWriters);

		// Act:
		auto sink = CreatePushEntitySink<TransactionSink>(locator, Service_Name);
		sink(transactionInfos);

		// Assert:
		ASSERT_EQ(1u, pWriters->broadcastedPayloads().size());
		EXPECT_EQ(ionet::PacketType::Push_Transactions, pWriters->broadcastedPayloads()[0].header().Type);
		test::AssertEqualPayload(expectedPayload, pWriters->broadcastedPayloads()[0]);
	}

	TEST(TEST_CLASS, CanCreatePushEntitySinkWithCustomPacketType) {
		// Arrange:
		auto pWriters = std::make_shared<mocks::BroadcastAwareMockPacketWriters>();
		auto keyPair = test::GenerateKeyPair();
		auto transactionInfos = test::CreateTransactionInfos(1);
		auto expectedPayload = ionet::CreateBroadcastPayload(transactionInfos, ionet::PacketType::Push_Partial_Transactions);
		ServiceLocator locator(keyPair);
		locator.registerService(Service_Name, pWriters);

		// Act:
		auto sink = CreatePushEntitySink<TransactionSink>(locator, Service_Name, ionet::PacketType::Push_Partial_Transactions);
		sink(transactionInfos);

		// Assert:
		ASSERT_EQ(1u, pWriters->broadcastedPayloads().size());
		EXPECT_EQ(ionet::PacketType::Push_Partial_Transactions, pWriters->broadcastedPayloads()[0].header().Type);
		test::AssertEqualPayload(expectedPayload, pWriters->broadcastedPayloads()[0]);
	}
}}
