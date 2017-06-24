#pragma once
#include "Node.h"
#include "PacketIo.h"
#include <memory>

namespace catapult { namespace ionet {

	/// A node and packet io pair.
	class NodePacketIoPair {
	public:
		/// Creates an empty pair.
		NodePacketIoPair()
		{}

		/// Creates a pair around \a node and \a pPacketIo.
		explicit NodePacketIoPair(const Node& node, const std::shared_ptr<PacketIo>& pPacketIo)
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
		operator bool() const {
			return !!m_pPacketIo;
		}

	private:
		Node m_node;
		std::shared_ptr<PacketIo> m_pPacketIo;
	};
}}
