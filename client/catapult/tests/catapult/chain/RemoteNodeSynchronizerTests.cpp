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

#include "catapult/chain/RemoteNodeSynchronizer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS RemoteNodeSynchronizerTests

	namespace {
		struct RemoteApi {};

		class MockSynchronizer {
		public:
			using RemoteApiType = RemoteApi;

		public:
			const auto& capturedRemoteApis() const {
				return m_capturedRemoteApis;
			}

		public:
			thread::future<ionet::NodeInteractionResultCode> operator()(const RemoteApi& remoteApi) {
				m_capturedRemoteApis.push_back(&remoteApi);
				return thread::make_ready_future(ionet::NodeInteractionResultCode::Success);
			}

		private:
			std::vector<const RemoteApi*> m_capturedRemoteApis;
		};
	}

	TEST(TEST_CLASS, FunctionDelegatesToSynchronizer) {
		// Arrange:
		auto pSynchronizer = std::make_shared<MockSynchronizer>();
		auto remoteNodeSynchronizer = CreateRemoteNodeSynchronizer(pSynchronizer);

		// Act:
		RemoteApi remoteApi;
		auto code = remoteNodeSynchronizer(remoteApi).get();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Success, code);
		ASSERT_EQ(1u, pSynchronizer->capturedRemoteApis().size());
		EXPECT_EQ(&remoteApi, pSynchronizer->capturedRemoteApis()[0]);
	}

	TEST(TEST_CLASS, ConditionalFunctionDelegatesToSynchronizer_WhenConditionReturnsTrue) {
		// Arrange:
		auto pSynchronizer = std::make_shared<MockSynchronizer>();
		auto remoteNodeSynchronizer = CreateConditionalRemoteNodeSynchronizer(pSynchronizer, []() { return true; });

		// Act:
		RemoteApi remoteApi;
		auto code = remoteNodeSynchronizer(remoteApi).get();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Success, code);
		ASSERT_EQ(1u, pSynchronizer->capturedRemoteApis().size());
		EXPECT_EQ(&remoteApi, pSynchronizer->capturedRemoteApis()[0]);
	}

	TEST(TEST_CLASS, ConditionalFunctionBypassesSynchronizer_WhenConditionReturnsFalse) {
		// Arrange:
		auto pSynchronizer = std::make_shared<MockSynchronizer>();
		auto remoteNodeSynchronizer = CreateConditionalRemoteNodeSynchronizer(pSynchronizer, []() { return false; });

		// Act:
		RemoteApi remoteApi;
		auto code = remoteNodeSynchronizer(remoteApi).get();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Neutral, code);
		EXPECT_EQ(0u, pSynchronizer->capturedRemoteApis().size());
	}
}}
