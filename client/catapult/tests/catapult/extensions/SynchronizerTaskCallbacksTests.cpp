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
			model::NodeIdentity FactoryRemoteIdentity;
			const model::TransactionRegistry* pFactoryTransactionRegistry = nullptr;

			size_t NumActionCalls = 0;
			int ActionApiId = 0;
		};

		template<typename TTraits>
		thread::TaskCallback ProcessSyncAndCapture(
				test::ServiceTestState& testState,
				net::PacketIoPicker& packetIoPicker,
				bool isChainSynced,
				TaskCallbackParamsCapture& capture,
				ionet::NodeInteractionResultCode code = ionet::NodeInteractionResultCode::Success) {
			const_cast<utils::TimeSpan&>(testState.config().Node.SyncTimeout) = utils::TimeSpan::FromSeconds(Default_Timeout_Seconds);

			testState.state().hooks().setChainSyncedPredicate([isChainSynced, &capture]() {
				++capture.NumChainSyncedCalls;
				return isChainSynced;
			});

			auto synchronizer = chain::RemoteNodeSynchronizer<int>([&capture, code](const auto& apiId) {
				++capture.NumActionCalls;
				capture.ActionApiId = apiId;
				return thread::make_ready_future(std::move(static_cast<ionet::NodeInteractionResultCode>(code)));
			});
			auto remoteApiFactory = [&capture](const auto& packetIo, const auto& remoteIdentity, const auto& registry) {
				++capture.NumFactoryCalls;
				capture.pFactoryPacketIo = &packetIo;
				capture.FactoryRemoteIdentity = remoteIdentity;
				capture.pFactoryTransactionRegistry = &registry;
				return std::make_unique<int>(Default_Action_Api_Id);
			};
			return TTraits::CreateTask(std::move(synchronizer), remoteApiFactory, packetIoPicker, testState.state(), "test");
		}

		struct DefaultCallbackTraits {
			static constexpr auto Num_Expected_Chain_Synced_Calls = 0u;

			template<typename... TArgs>
			static auto CreateTask(TArgs&&... args) {
				return extensions::CreateSynchronizerTaskCallback(std::forward<TArgs>(args)...);
			}
		};

		struct ChainSyncAwareCallbackTraits {
			static constexpr auto Num_Expected_Chain_Synced_Calls = 1u;

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
			EXPECT_EQ(TTraits::Num_Expected_Chain_Synced_Calls, capture.NumChainSyncedCalls);

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
			auto identityKey = test::GenerateRandomByteArray<Key>();
			mocks::PickOneAwareMockPacketWriters writers;
			writers.setPacketIo(pPacketIo);
			writers.setNodeIdentity({ identityKey, "11.22.33.44" });

			// Act:
			TaskCallbackParamsCapture capture;
			auto result = ProcessSyncAndCapture<TTraits>(testState, writers, isChainSynced, capture)().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);

			// - chain synced was called expected number of times
			EXPECT_EQ(TTraits::Num_Expected_Chain_Synced_Calls, capture.NumChainSyncedCalls);

			// - pick one was called
			ASSERT_EQ(1u, writers.numPickOneCalls());
			EXPECT_EQ(Default_Timeout_Seconds, writers.pickOneDurations()[0].seconds());

			// - factory was called
			EXPECT_EQ(1u, capture.NumFactoryCalls);
			EXPECT_EQ(pPacketIo.get(), capture.pFactoryPacketIo);
			EXPECT_EQ(identityKey, capture.FactoryRemoteIdentity.PublicKey);
			EXPECT_EQ("11.22.33.44", capture.FactoryRemoteIdentity.Host);
			EXPECT_EQ(&testState.state().pluginManager().transactionRegistry(), capture.pFactoryTransactionRegistry);

			// - action was called
			EXPECT_EQ(1u, capture.NumActionCalls);
			EXPECT_EQ(Default_Action_Api_Id, capture.ActionApiId);
		}
	}

	TEST(TEST_CLASS, DefaultCallback_ActionIsSkippedWhenNoPeerIsAvailable) {
		AssertActionIsSkippedWhenNoPeerIsAvailable<DefaultCallbackTraits>();
	}

	TEST(TEST_CLASS, DefaultCallback_ActionIsCalledWhenPeerIsAvailableAndChainIsNotSynched) {
		AssertCallbackCallsAction<DefaultCallbackTraits>(false);
	}

	TEST(TEST_CLASS, DefaultCallback_ActionIsCalledWhenPeerIsAvailableAndChainIsSynched) {
		AssertCallbackCallsAction<DefaultCallbackTraits>(true);
	}

	TEST(TEST_CLASS, ChainSyncedCallback_ActionIsSkippedWhenNoPeerIsAvailable) {
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
		AssertCallbackCallsAction<ChainSyncAwareCallbackTraits>(true);
	}

	namespace {
		template<typename TAssert>
		void AssertNodeInteractionResultIsInspected(ionet::NodeInteractionResultCode code, TAssert assertFunc) {
			// Arrange:
			test::ServiceTestState testState;
			auto nodeIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
			auto pPacketIo = std::make_shared<mocks::MockPacketIo>();
			mocks::PickOneAwareMockPacketWriters writers;
			writers.setPacketIo(pPacketIo);
			writers.setNodeIdentity(nodeIdentity);

			{
				auto nodesModifier = testState.state().nodes().modifier();
				nodesModifier.add(ionet::Node(nodeIdentity), ionet::NodeSource::Dynamic);
			}

			// Act:
			TaskCallbackParamsCapture capture;
			auto result = ProcessSyncAndCapture<DefaultCallbackTraits>(testState, writers, true, capture, code)().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);

			auto interactions = testState.state().nodes().view().getNodeInfo(nodeIdentity).interactions(Timestamp());
			assertFunc(interactions);
		}

		void AssertNodeInteractionsAreNotUpdated(ionet::NodeInteractionResultCode code) {
			AssertNodeInteractionResultIsInspected(code, [](const auto& interactions) {
				test::AssertNodeInteractions(0, 0, interactions);
			});
		}
	}

	TEST(TEST_CLASS, NodeInteractionsAreUpdatedOnSuccessfulInteraction) {
		AssertNodeInteractionResultIsInspected(ionet::NodeInteractionResultCode::Success, [](const auto& interactions) {
			test::AssertNodeInteractions(1, 0, interactions);
		});
	}

	TEST(TEST_CLASS, NodeInteractionsAreUpdatedOnFailedInteraction) {
		AssertNodeInteractionResultIsInspected(ionet::NodeInteractionResultCode::Failure, [](const auto& interactions) {
			test::AssertNodeInteractions(0, 1, interactions);
		});
	}

	TEST(TEST_CLASS, NodeInteractionsAreNotUpdatedOnNeutralInteraction) {
		AssertNodeInteractionsAreNotUpdated(ionet::NodeInteractionResultCode::Neutral);
	}

	TEST(TEST_CLASS, NodeInteractionsAreNotUpdatedOnNoneInteraction) {
		AssertNodeInteractionsAreNotUpdated(ionet::NodeInteractionResultCode::None);
	}
}}
