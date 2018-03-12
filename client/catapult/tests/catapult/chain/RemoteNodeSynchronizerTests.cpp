#include "catapult/chain/RemoteNodeSynchronizer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS RemoteNodeSynchronizerTests

	namespace {
		struct RemoteApi {
		};

		class MockSynchronizer {
		public:
			using RemoteApiType = RemoteApi;

		public:
			const auto& capturedRemoteApis() const {
				return m_capturedRemoteApis;
			}

		public:
			thread::future<NodeInteractionResult> operator()(const RemoteApi& remoteApi) {
				m_capturedRemoteApis.push_back(&remoteApi);
				return thread::make_ready_future(NodeInteractionResult::Neutral);
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
		auto result = remoteNodeSynchronizer(remoteApi).get();

		// Assert:
		EXPECT_EQ(NodeInteractionResult::Neutral, result);
		ASSERT_EQ(1u, pSynchronizer->capturedRemoteApis().size());
		EXPECT_EQ(&remoteApi, pSynchronizer->capturedRemoteApis()[0]);
	}
}}
