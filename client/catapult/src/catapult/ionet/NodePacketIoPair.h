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
#include "Node.h"
#include "PacketIo.h"
#include <memory>

namespace catapult { namespace ionet {

	/// Node and packet io pair.
	class NodePacketIoPair {
	public:
		/// Creates an empty pair.
		NodePacketIoPair()
		{}

		/// Creates a pair around \a node and \a pPacketIo.
		NodePacketIoPair(const Node& node, const std::shared_ptr<PacketIo>& pPacketIo)
				: m_node(node)
				, m_pPacketIo(pPacketIo)
		{}

	public:
		/// Gets the node.
		const Node& node() const {
			return m_node;
		}

		/// Gets the io.
		const std::shared_ptr<PacketIo>& io() const {
			return m_pPacketIo;
		}

	public:
		/// Returns \c true if this pair is not empty.
		explicit operator bool() const {
			return !!m_pPacketIo;
		}

	private:
		Node m_node;
		std::shared_ptr<PacketIo> m_pPacketIo;
	};
}}
