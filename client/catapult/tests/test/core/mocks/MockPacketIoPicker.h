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

#pragma once
#include "MockPacketIo.h"
#include "catapult/net/PacketIoPicker.h"
#include <list>

namespace catapult { namespace mocks {

	/// Mock PacketIoPicker that can be prepared with MockPacketIos.
	/// \note Returned node names are deterministic in the range [1..N].
	class MockPacketIoPicker : public net::PacketIoPicker {
	public:
		/// Creates a mock packet io picker that initially has \a numPacketIos mock packet ios available.
		explicit MockPacketIoPicker(size_t numPacketIos) : m_nextIndex(0) {
			for (auto i = 0u; i < numPacketIos; ++i)
				m_packetIos.push_back(std::make_shared<mocks::MockPacketIo>());
		}

	public:
		/// Gets the durations that were captured.
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

			auto node = ionet::Node({}, {}, { model::UniqueNetworkFingerprint(), std::to_string(m_nextIndex) });
			return ionet::NodePacketIoPair(node, m_packetIos[m_nextIndex - 1]);
		}

	private:
		size_t m_nextIndex;
		std::vector<std::shared_ptr<mocks::MockPacketIo>> m_packetIos;
		std::vector<utils::TimeSpan> m_durations;
	};
}}
