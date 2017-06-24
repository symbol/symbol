#pragma once
#include "catapult/net/PacketIoPicker.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include <list>

namespace catapult { namespace mocks {

	/// A mock PacketIoPicker that can be prepared with MockPacketIos.
	/// \note Returned node names are deterministic in the range [1..N].
	class MockPacketIoPicker : public net::PacketIoPicker {
	public:
		/// Creates a mock packet io picker that initially has \a numPacketIos mock packet ios available.
		explicit MockPacketIoPicker(size_t numPacketIos) : m_nextIndex(0) {
			for (auto i = 0u; i < numPacketIos; ++i)
				m_packetIos.push_back(std::make_shared<mocks::MockPacketIo>());
		}

	public:
		/// Returns the durations that were captured.
		const std::vector<utils::TimeSpan>& durations() const {
			return m_durations;
		}

		/// Inserts a (mock) packet io (\a pPacketIo) into the container.
		void insert(const std::shared_ptr<mocks::MockPacketIo>& pPacketIo) {
			m_packetIos.push_back(pPacketIo);
		}

	public:
		ionet::NodePacketIoPair pickOne(const utils::TimeSpan& ioDuration) override {
			m_durations.push_back(ioDuration);
			if (m_nextIndex++ >= m_packetIos.size())
				return ionet::NodePacketIoPair();

			auto node = ionet::Node({}, { {}, std::to_string(m_nextIndex) }, model::NetworkIdentifier::Zero);
			return ionet::NodePacketIoPair(node, m_packetIos[m_nextIndex - 1]);
		}

	private:
		size_t m_nextIndex;
		std::vector<std::shared_ptr<mocks::MockPacketIo>> m_packetIos;
		std::vector<utils::TimeSpan> m_durations;
	};
}}
