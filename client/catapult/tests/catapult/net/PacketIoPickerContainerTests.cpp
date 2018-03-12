#include "catapult/net/PacketIoPickerContainer.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

#define TEST_CLASS PacketIoPickerContainerTests

	namespace {
		std::vector<std::shared_ptr<mocks::MockPacketIo>> CreateMockPacketIos(size_t numPacketIos) {
			std::vector<std::shared_ptr<mocks::MockPacketIo>> packetIos;
			for (auto i = 0u; i < numPacketIos; ++i)
				packetIos.push_back(std::make_shared<mocks::MockPacketIo>());

			return packetIos;
		}

		void AssertPickerCalls(
				const std::vector<mocks::PickOneAwareMockPacketWriters>& pickers,
				const std::unordered_set<size_t>& expectedUsedPickerIndexes,
				const utils::TimeSpan& expectedDuration) {
			auto i = 0u;
			for (const auto& picker : pickers) {
				auto message = "picker at " + std::to_string(i);
				bool isUseExpected = expectedUsedPickerIndexes.cend() != expectedUsedPickerIndexes.find(i);

				if (!isUseExpected) {
					EXPECT_EQ(0u, picker.numPickOneCalls()) << message;
				} else {
					EXPECT_EQ(1u, picker.numPickOneCalls()) << message;
					EXPECT_EQ(std::vector<utils::TimeSpan>({ expectedDuration }), picker.pickOneDurations()) << message;
				}

				++i;
			}
		}
	}

	TEST(TEST_CLASS, PickMatchingReturnsNoPairsWhenContainerIsEmpty) {
		// Arrange:
		PacketIoPickerContainer container;

		// Act:
		auto ioPairs = container.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::None);

		// Assert:
		EXPECT_TRUE(ioPairs.empty());
	}

	TEST(TEST_CLASS, PickMatchingReturnsPacketIosFromPacketPickers) {
		// Arrange:
		std::vector<mocks::PickOneAwareMockPacketWriters> pickers(4);
		auto packetIos = CreateMockPacketIos(4);
		for (auto i = 0u; i < pickers.size(); ++i)
			pickers[i].setPacketIo(packetIos[i]);

		PacketIoPickerContainer container;
		for (auto& picker : pickers)
			container.insert(picker, ionet::NodeRoles::None);

		// Act:
		auto ioPairs = container.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::None);

		// Assert: all pickers should be used
		ASSERT_EQ(4u, ioPairs.size());
		for (auto i = 0u; i < pickers.size(); ++i)
			EXPECT_EQ(packetIos[i], ioPairs[i].io()) << "result at " << i;

		AssertPickerCalls(pickers, { 0, 1, 2, 3 }, utils::TimeSpan::FromSeconds(1));
	}

	TEST(TEST_CLASS, PickMatchingReturnsPacketIosFromPacketPickersWithCompatibleRoles) {
		// Arrange:
		std::vector<mocks::PickOneAwareMockPacketWriters> pickers(4);
		auto packetIos = CreateMockPacketIos(4);
		for (auto i = 0u; i < pickers.size(); ++i)
			pickers[i].setPacketIo(packetIos[i]);

		PacketIoPickerContainer container;
		container.insert(pickers[0], ionet::NodeRoles::Api | ionet::NodeRoles::Peer);
		container.insert(pickers[1], ionet::NodeRoles::None);
		container.insert(pickers[2], ionet::NodeRoles::Api);
		container.insert(pickers[3], ionet::NodeRoles::Peer);

		// Act:
		auto ioPairs = container.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::Api);

		// Assert: only pickers with compatible roles should be used
		ASSERT_EQ(2u, ioPairs.size());
		EXPECT_EQ(packetIos[0], ioPairs[0].io());
		EXPECT_EQ(packetIos[2], ioPairs[1].io());

		AssertPickerCalls(pickers, { 0, 2 }, utils::TimeSpan::FromSeconds(1));
	}

	TEST(TEST_CLASS, PickMatchingReturnsPacketIosFromPacketPickersAndFiltersOutEmptyResults) {
		// Arrange:
		std::vector<mocks::PickOneAwareMockPacketWriters> pickers(4);
		auto packetIos = CreateMockPacketIos(4);
		for (auto i = 0u; i < pickers.size(); ++i)
			pickers[i].setPacketIo(0 == i % 2 ? nullptr : packetIos[i]);

		PacketIoPickerContainer container;
		for (auto& picker : pickers)
			container.insert(picker, ionet::NodeRoles::None);

		// Act:
		auto ioPairs = container.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::None);

		// Assert: all pickers should be used, but nullptr packet ios should be filtered out of results
		ASSERT_EQ(2u, ioPairs.size());
		EXPECT_EQ(packetIos[1], ioPairs[0].io());
		EXPECT_EQ(packetIos[3], ioPairs[1].io());

		AssertPickerCalls(pickers, { 0, 1, 2, 3 }, utils::TimeSpan::FromSeconds(1));
	}
}}
