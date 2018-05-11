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

#include "catapult/extensions/SynchronizerTaskCallbacks.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS SynchronizerTaskCallbacksTests

	namespace {
		constexpr auto Default_Action_Api_Id = 7;
		constexpr auto Default_Timeout_Seconds = 3u;

		struct TaskCallbackParamsCapture {
			size_t NumChainSyncedCalls = 0;

			size_t NumFactoryCalls = 0;
			const ionet::PacketIo* pFactoryPacketIo = nullptr;
			const model::TransactionRegistry* pFactoryTransactionRegistry = nullptr;

			size_t NumActionCalls = 0;
			int ActionApiId = 0;
		};

		template<typename TTraits>
		thread::TaskCallback ProcessSyncAndCapture(
				test::ServiceTestState& testState,
				net::PacketIoPicker& packetIoPicker,
				bool isChainSynced,
				TaskCallbackParamsCapture& capture) {
			const_cast<utils::TimeSpan&>(testState.config().Node.SyncTimeout) = utils::TimeSpan::FromSeconds(Default_Timeout_Seconds);

			testState.state().hooks().setChainSyncedPredicate([isChainSynced, &capture]() {
				++capture.NumChainSyncedCalls;
				return isChainSynced;
			});

			return TTraits::CreateTask(
				chain::RemoteNodeSynchronizer<int>([&capture](const auto& apiId) {
					++capture.NumActionCalls;
					capture.ActionApiId = apiId;
					return thread::make_ready_future(chain::NodeInteractionResult::Success);
				}),
				[&capture](const auto& packetIo, const auto& registry) {
					++capture.NumFactoryCalls;
					capture.pFactoryPacketIo = &packetIo;
					capture.pFactoryTransactionRegistry = &registry;
					return std::make_unique<int>(Default_Action_Api_Id);
				},
				packetIoPicker,
				testState.state(),
				"test");
		}

		struct DefaultCallbackTraits {
			static constexpr auto NumExpectedChainSyncedCalls() { return 0u; }

			template<typename... TArgs>
			static auto CreateTask(TArgs&&... args) {
				return extensions::CreateSynchronizerTaskCallback(std::forward<TArgs>(args)...);
			}
		};

		struct ChainSyncAwareCallbackTraits {
			static constexpr auto NumExpectedChainSyncedCalls() { return 1u; }

			template<typename... TArgs>
			static auto CreateTask(TArgs&&... args) {
				return extensions::CreateChainSyncAwareSynchronizerTaskCallback(std::forward<TArgs>(args)...);
			}
		};

		template<typename TTraits>
		void AssertActionIsSkippedWhenNoPeerIsAvailable() {
			// Arrange: create an empty writers
			test::ServiceTestState testState;
			mocks::PickOneAwareMockPacketWriters writers;

			// Act:
			TaskCallbackParamsCapture capture;
			auto result = ProcessSyncAndCapture<TTraits>(testState, writers, true, capture)().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);

			// - chain synced was called expected number of times
			EXPECT_EQ(TTraits::NumExpectedChainSyncedCalls(), capture.NumChainSyncedCalls);

			// - pick one was called
			ASSERT_EQ(1u, writers.numPickOneCalls());
			EXPECT_EQ(Default_Timeout_Seconds, writers.pickOneDurations()[0].seconds());

			// - other calls were bypassed
			EXPECT_EQ(0u, capture.NumFactoryCalls);
			EXPECT_EQ(0u, capture.NumActionCalls);
		}

		template<typename TTraits>
		void AssertCallbackCallsAction(bool isChainSynced) {
			// Arrange: create writers with a valid packet
			test::ServiceTestState testState;
			auto pPacketIo = std::make_shared<mocks::MockPacketIo>();
			mocks::PickOneAwareMockPacketWriters writers;
			writers.setPacketIo(pPacketIo);

			// Act:
			TaskCallbackParamsCapture capture;
			auto result = ProcessSyncAndCapture<TTraits>(testState, writers, isChainSynced, capture)().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);

			// - chain synced was called expected number of times
			EXPECT_EQ(TTraits::NumExpectedChainSyncedCalls(), capture.NumChainSyncedCalls);

			// - pick one was called
			ASSERT_EQ(1u, writers.numPickOneCalls());
			EXPECT_EQ(Default_Timeout_Seconds, writers.pickOneDurations()[0].seconds());

			// - factory was called
			EXPECT_EQ(1u, capture.NumFactoryCalls);
			EXPECT_EQ(pPacketIo.get(), capture.pFactoryPacketIo);
			EXPECT_EQ(&testState.state().pluginManager().transactionRegistry(), capture.pFactoryTransactionRegistry);

			// - action was called
			EXPECT_EQ(1u, capture.NumActionCalls);
			EXPECT_EQ(Default_Action_Api_Id, capture.ActionApiId);
		}
	}

	TEST(TEST_CLASS, DefaultCallback_ActionIsSkippedWhenNoPeerIsAvailable) {
		// Assert:
		AssertActionIsSkippedWhenNoPeerIsAvailable<DefaultCallbackTraits>();
	}

	TEST(TEST_CLASS, DefaultCallback_ActionIsCalledWhenPeerIsAvailableAndChainIsNotSynched) {
		// Assert:
		AssertCallbackCallsAction<DefaultCallbackTraits>(false);
	}

	TEST(TEST_CLASS, DefaultCallback_ActionIsCalledWhenPeerIsAvailableAndChainIsSynched) {
		// Assert:
		AssertCallbackCallsAction<DefaultCallbackTraits>(true);
	}

	TEST(TEST_CLASS, ChainSyncedCallback_ActionIsSkippedWhenNoPeerIsAvailable) {
		// Assert:
		AssertActionIsSkippedWhenNoPeerIsAvailable<ChainSyncAwareCallbackTraits>();
	}

	TEST(TEST_CLASS, ChainSyncedCallback_ActionIsSkippedWhenPeerIsAvailableAndChainIsNotSynched) {
		// Arrange: create writers with a valid packet
		test::ServiceTestState testState;
		auto pPacketIo = std::make_shared<mocks::MockPacketIo>();
		mocks::PickOneAwareMockPacketWriters writers;
		writers.setPacketIo(pPacketIo);

		// Act:
		TaskCallbackParamsCapture capture;
		auto result = ProcessSyncAndCapture<ChainSyncAwareCallbackTraits>(testState, writers, false, capture)().get();

		// Assert:
		EXPECT_EQ(thread::TaskResult::Continue, result);

		// - chain synced was called once
		EXPECT_EQ(1u, capture.NumChainSyncedCalls);

		// - pick one was bypassed
		EXPECT_EQ(0u, writers.numPickOneCalls());

		// - other calls were bypassed
		EXPECT_EQ(0u, capture.NumFactoryCalls);
		EXPECT_EQ(0u, capture.NumActionCalls);
	}

	TEST(TEST_CLASS, ChainSyncedCallback_ActionIsCalledWhenPeerIsAvailableAndChainIsSynched) {
		// Assert:
		AssertCallbackCallsAction<ChainSyncAwareCallbackTraits>(true);
	}
}}
