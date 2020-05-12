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

#include "timesync/src/TimeSynchronizationService.h"
#include "timesync/src/TimeSynchronizationConfiguration.h"
#include "timesync/src/TimeSynchronizationState.h"
#include "timesync/src/api/TimeSyncPackets.h"
#include "timesync/src/types.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/ionet/PacketSocket.h"
#include "timesync/tests/test/TimeSynchronizationCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/net/BriefServerRequestorTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps//Waits.h"
#include "tests/TestHarness.h"

namespace catapult { namespace timesync {

#define TEST_CLASS TimeSynchronizationServiceTests

	namespace {
		constexpr auto Num_Expected_Counters = 4u;
		constexpr auto Time_Offset_Absolute_Counter_Name = "TS OFFSET ABS";
		constexpr auto Time_Offset_Direction_Counter_Name = "TS OFFSET DIR";
		constexpr auto Node_Age_Counter_Name = "TS NODE AGE";
		constexpr auto Total_Requests_Counter_Name = "TS TOTAL REQ";
		constexpr auto Sentinel_Counter_Value = extensions::ServiceLocator::Sentinel_Counter_Value;

		constexpr auto Num_Expected_Services = 3u;
		constexpr auto Requestor_Service_Name = "timesync.requestor";
		constexpr auto Synchronizer_Service_Name = "timesync.synchronizer";
		constexpr auto State_Service_Name = "timesync.state";
		constexpr auto Task_Name = "time synchronization task";

		constexpr auto Default_Epoch_Adjustment = utils::TimeSpan::FromMilliseconds(11223344556677);
		constexpr uint64_t Default_Threshold = 85;

		cache::CatapultCache CreateCache(Importance totalChainImportance) {
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.ImportanceGrouping = 123;
			blockChainConfig.TotalChainImportance = totalChainImportance;
			return test::CoreSystemCacheFactory::Create(blockChainConfig);
		}

		cache::CatapultCache CreateCache() {
			return CreateCache(Importance());
		}

		struct TimeSynchronizationServiceTraits {
			static auto CreateRegistrar(
					const TimeSynchronizationConfiguration& timeSyncConfig,
					const std::shared_ptr<TimeSynchronizationState>& pTimeSyncState) {
				return CreateTimeSynchronizationServiceRegistrar(timeSyncConfig, pTimeSyncState);
			}

			static auto CreateRegistrar(const std::shared_ptr<TimeSynchronizationState>& pTimeSyncState) {
				auto config = TimeSynchronizationConfiguration::Uninitialized();
				config.MaxNodes = 5;
				return CreateRegistrar(config, pTimeSyncState);
			}

			static auto CreateRegistrar() {
				return CreateRegistrar(std::make_shared<TimeSynchronizationState>(Default_Epoch_Adjustment, Default_Threshold));
			}
		};

		using TestContext = test::ServiceLocatorTestContext<TimeSynchronizationServiceTraits>;
	}

	// region basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(TimeSynchronization, Post_Packet_Io_Pickers)

	TEST(TEST_CLASS, CanBootService) {
		// Arrange:
		TestContext context(CreateCache());

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Expected_Services, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());

		EXPECT_TRUE(!!context.locator().service<void>(Requestor_Service_Name));
		EXPECT_TRUE(!!context.locator().service<void>(Synchronizer_Service_Name));
		EXPECT_TRUE(!!context.locator().service<void>(State_Service_Name));

		EXPECT_EQ(0u, context.counter(Time_Offset_Absolute_Counter_Name));
		EXPECT_EQ(0u, context.counter(Time_Offset_Direction_Counter_Name));
		EXPECT_EQ(0u, context.counter(Node_Age_Counter_Name));
		EXPECT_EQ(0u, context.counter(Total_Requests_Counter_Name));
	}

	TEST(TEST_CLASS, CanShutdownService) {
		// Arrange:
		TestContext context(CreateCache());
		context.boot();

		// Act:
		context.shutdown();

		// Assert:
		EXPECT_EQ(Num_Expected_Services, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());

		EXPECT_FALSE(!!context.locator().service<void>(Requestor_Service_Name));
		EXPECT_TRUE(!!context.locator().service<void>(Synchronizer_Service_Name));
		EXPECT_TRUE(!!context.locator().service<void>(State_Service_Name));

		EXPECT_EQ(0u, context.counter(Time_Offset_Absolute_Counter_Name));
		EXPECT_EQ(0u, context.counter(Time_Offset_Direction_Counter_Name));
		EXPECT_EQ(0u, context.counter(Node_Age_Counter_Name));
		EXPECT_EQ(Sentinel_Counter_Value, context.counter(Total_Requests_Counter_Name));
	}

	TEST(TEST_CLASS, PacketHandlersAreRegistered) {
		// Arrange:
		TestContext context(CreateCache());

		// Act:
		context.boot();
		const auto& handlers = context.testState().state().packetHandlers();

		// Assert:
		EXPECT_EQ(1u, handlers.size());
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Time_Sync_Network_Time));
	}

	TEST(TEST_CLASS, TasksAreRegistered) {
		// Arrange:
		TestContext context(CreateCache());

		// Act:
		context.boot();
		const auto& tasks = context.testState().state().tasks();

		// Assert:
		EXPECT_EQ(1u, tasks.size());
		EXPECT_EQ(Task_Name, tasks.cbegin()->Name);
	}

	TEST(TEST_CLASS, ServiceUsesNetworkTime) {
		// Arrange:
		auto pTimeSyncState = std::make_shared<TimeSynchronizationState>(Default_Epoch_Adjustment, 100);
		pTimeSyncState->update(TimeOffset(500));
		TestContext context(CreateCache(), [&timeSyncState = *pTimeSyncState]() {
			return timeSyncState.networkTime();
		});
		context.boot();
		const auto timeSupplier = context.testState().state().timeSupplier();
		Timestamp timestamp;
		Timestamp networkTime;

		// Act:
		test::RunDeterministicOperation([timeSupplier, &timestamp, &networkTime]() {
			timestamp = utils::NetworkTime(Default_Epoch_Adjustment).now();
			networkTime = timeSupplier();
		});

		// Assert:
		EXPECT_EQ(timestamp + Timestamp(500), networkTime);
	}

	// endregion

	// region current offset

	namespace {
		constexpr uint8_t Positive = 0u;
		constexpr Importance Total_Chain_Importance(1'000'000);

		auto CreateValidResponsePacket(uint64_t timeOffset) {
			auto pResponsePacket = ionet::CreateSharedPacket<api::NetworkTimePacket>();
			// local timestamps are 0 and 800, so roundtrip time is 800
			pResponsePacket->CommunicationTimestamps.SendTimestamp = Timestamp(400 + timeOffset);
			pResponsePacket->CommunicationTimestamps.ReceiveTimestamp = Timestamp(400 + timeOffset);
			return pResponsePacket;
		}

		class NetworkTimeServer : public test::RemotePullServer {
		public:
			void prepareValidResponse(int64_t timeOffset) {
				auto pResponsePacket = CreateValidResponsePacket(static_cast<uint64_t>(timeOffset));
				test::RemotePullServer::prepareValidResponse(pResponsePacket);
			}
		};

		enum class ResponseType { Success, Error };

		template<typename TAssertState>
		void AssertStateChange(int64_t remoteOffset, Importance importance, ResponseType responseType, TAssertState assertState) {
			// Arrange: prepare account state cache
			NetworkTimeServer networkTimeServer;
			auto serverPublicKey = networkTimeServer.caPublicKey();
			auto cache = CreateCache(Total_Chain_Importance);
			{
				auto cacheDelta = cache.createDelta();
				auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
				test::AddAccount(accountStateCacheDelta, serverPublicKey, importance, model::ImportanceHeight(1));
				accountStateCacheDelta.updateHighValueAccounts(Height(1));
				cache.commit(Height(1));
			}

			// - simulate the remote node by responding with communication timestamps
			if (ResponseType::Success == responseType)
				networkTimeServer.prepareValidResponse(remoteOffset);
			else
				networkTimeServer.prepareNoResponse();

			// - use a deterministic time supplier to make the tests deterministic
			auto numTimeSuppierCalls = 0u;
			auto timeSupplier = [&numTimeSuppierCalls]() { return Timestamp(numTimeSuppierCalls++ * 800); };

			// - prepare context
			TestContext context(std::move(cache), timeSupplier);
			auto& blockChainConfig = const_cast<model::BlockChainConfiguration&>(context.testState().config().BlockChain);
			blockChainConfig.TotalChainImportance = Total_Chain_Importance;
			test::AddNode(context.testState().state().nodes(), serverPublicKey, "alice");
			auto pTimeSyncState = std::make_shared<TimeSynchronizationState>(Default_Epoch_Adjustment, Default_Threshold);
			context.boot(pTimeSyncState);

			// Sanity:
			EXPECT_EQ(0u, context.counter(Time_Offset_Absolute_Counter_Name));
			EXPECT_EQ(Positive, context.counter(Time_Offset_Direction_Counter_Name));
			EXPECT_EQ(0u, context.counter(Node_Age_Counter_Name));
			EXPECT_EQ(0u, context.counter(Total_Requests_Counter_Name));

			// Act:
			test::RunTaskTestPostBoot(context, 1u, Task_Name, [&context, &networkTimeServer, responseType, assertState](const auto& task) {
				auto future = task.Callback();

				if (ResponseType::Error == responseType) {
					WAIT_FOR_EXPR(networkTimeServer.hasConnection());

					// Act: close the connection
					networkTimeServer.close();
				}

				auto result = future.get();

				// Assert:
				EXPECT_EQ(thread::TaskResult::Continue, result);
				assertState(context);
			});
		}
	}

	TEST(TEST_CLASS, TaskExecutionDoesNotChangeOffsetWhenChangeIsLessThanThreshold) {
		// Assert: importance = 0.1, calculated offset = 0.1 * 500 = 50 and threshold is 85
		AssertStateChange(500, Importance(100'000), ResponseType::Success, [](const auto& context) {
			EXPECT_EQ(0u, context.counter(Time_Offset_Absolute_Counter_Name));
			EXPECT_EQ(Positive, context.counter(Time_Offset_Direction_Counter_Name));
			EXPECT_EQ(1u, context.counter(Node_Age_Counter_Name));
			EXPECT_EQ(1u, context.counter(Total_Requests_Counter_Name));
		});
	}

	TEST(TEST_CLASS, TaskExecutionChangesOffsetWhenChangeIsGreaterThanThreshold) {
		// Assert: importance = 0.5, calculated offset = 0.5 * 200 = 100 and threshold is 85
		AssertStateChange(200, Importance(500'000), ResponseType::Success, [](const auto& context) {
			EXPECT_EQ(100u, context.counter(Time_Offset_Absolute_Counter_Name));
			EXPECT_EQ(Positive, context.counter(Time_Offset_Direction_Counter_Name));
			EXPECT_EQ(1u, context.counter(Node_Age_Counter_Name));
			EXPECT_EQ(1u, context.counter(Total_Requests_Counter_Name));
		});
	}

	TEST(TEST_CLASS, TaskExecutionDoesNotChangeOffsetWhenRemoteErrors) {
		// Assert: initial offset is zero
		AssertStateChange(200, Importance(500'000), ResponseType::Error, [](const auto& context) {
			EXPECT_EQ(0u, context.counter(Time_Offset_Absolute_Counter_Name));
			EXPECT_EQ(Positive, context.counter(Time_Offset_Direction_Counter_Name));
			EXPECT_EQ(1u, context.counter(Node_Age_Counter_Name));
			EXPECT_EQ(1u, context.counter(Total_Requests_Counter_Name));
		});
	}

	// endregion
}}
