#include "catapult/local/p2p/LocalNodeFunctionalPlugins.h"
#include "catapult/api/ChainPackets.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/test/core/mocks/MockPacketIoPicker.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

	// region RemoteChainHeightsRetriever

	namespace {
		using ChainInfoPacket = api::ChainInfoResponse;

		auto CreateMockPacketIoForHeight(Height height) {
			auto pPacketIo = std::make_shared<mocks::MockPacketIo>();
			pPacketIo->queueWrite(ionet::SocketOperationCode::Success);
			pPacketIo->queueRead(ionet::SocketOperationCode::Success, [height](const auto*) -> std::shared_ptr<ChainInfoPacket> {
				auto pPacket = ionet::CreateSharedPacket<ChainInfoPacket>();
				pPacket->Height = height;
				return pPacket;
			});
			return pPacketIo;
		}

		void AssertEqual(const std::vector<Height>& expectedHeights, const std::vector<Height>& actualHeights) {
			ASSERT_EQ(expectedHeights.size(), actualHeights.size());

			auto i = 0u;
			for (auto height : expectedHeights) {
				EXPECT_EQ(height, actualHeights[i]) << "height at " << i;
				++i;
			}
		}

		void AssertReturnedHeights(size_t numPeers, size_t numPacketIos, const std::vector<Height>& expectedHeights) {
			// Arrange:
			mocks::MockPacketIoPicker picker(0);
			for (auto i = 0u; i < numPacketIos; ++i)
				picker.insert(CreateMockPacketIoForHeight(Height(i + 1)));

			auto retriever = CreateRemoteChainHeightsRetriever(picker);

			// Act:
			auto heights = retriever(numPeers).get();

			// Assert:
			AssertEqual(expectedHeights, heights);
		}
	}

	TEST(LocalNodeFunctionalPluginsTests, RemoteChainHeightsRetriever_ReturnsEmptyHeightVectorIfNoPacketIosAreAvailable) {
		// Assert:
		AssertReturnedHeights(3, 0, {});
	}

	TEST(LocalNodeFunctionalPluginsTests, RemoteChainHeightsRetriever_ReturnsLessThanNumPeersHeightsIfNotEnoughPacketIosAreAvailable) {
		// Assert:
		AssertReturnedHeights(3, 1, { { Height(1) } });
		AssertReturnedHeights(3, 2, { { Height(1), Height(2) } });
	}

	TEST(LocalNodeFunctionalPluginsTests, RemoteChainHeightsRetriever_ReturnsNumPeersHeightsIfEnoughPacketIosAreAvailable) {
		// Assert:
		for (auto i = 3u; i < 10; ++i)
			AssertReturnedHeights(i, 3, { { Height(1), Height(2), Height(3) } });
	}

	// endregion

	// region CreateChainSyncedPredicate

	namespace {
		void AssertChainSyncedPredicate(uint32_t localChainHeight, uint32_t remoteChainHeight, bool expectedResult) {
			// Arrange:
			auto pStorage = mocks::CreateMemoryBasedStorageCache(localChainHeight);
			NetworkChainHeightSupplier networkChainHeightSupplier = [remoteChainHeight]() { return remoteChainHeight; };
			auto predicate = CreateChainSyncedPredicate(*pStorage, networkChainHeightSupplier);

			// Act:
			auto result = predicate();

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(LocalNodeFunctionalPluginsTests, ChainSyncedPredicate_ReturnsTrueIfLocalChainHeightIsWithinAcceptedRange) {
		// Assert:
		AssertChainSyncedPredicate(100, 23, true);
		AssertChainSyncedPredicate(24, 23, true);
		AssertChainSyncedPredicate(23, 23, true);
		AssertChainSyncedPredicate(23, 24, true);
		AssertChainSyncedPredicate(23, 25, true);
		AssertChainSyncedPredicate(23, 26, true);
	}

	TEST(LocalNodeFunctionalPluginsTests, ChainSyncedPredicate_ReturnsFalseIfLocalChainHeightIsOutsideOfAcceptedRange) {
		// Assert:
		AssertChainSyncedPredicate(23, 27, false);
		AssertChainSyncedPredicate(23, 28, false);
		AssertChainSyncedPredicate(23, 100, false);
		AssertChainSyncedPredicate(23, 1000, false);
	}

	// endregion
}}}
