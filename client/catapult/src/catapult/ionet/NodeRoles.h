#pragma once
#include "catapult/utils/BitwiseEnum.h"
#include <string>

namespace catapult { namespace ionet {

	/// A node's role.
	enum class NodeRoles : uint32_t {
		/// No roles.
		None,

		/// A peer node.
		Peer = 0x01,

		/// An api node.
		Api = 0x02
	};

	MAKE_BITWISE_ENUM(NodeRoles);

	/// Tries to parse \a str into node \a roles.
	bool TryParseValue(const std::string& str, NodeRoles& roles);
}}
