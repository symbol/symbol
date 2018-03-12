#include "NodeRoles.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/ConfigurationValueParsers.h"

namespace catapult { namespace ionet {

	const std::array<std::pair<const char*, NodeRoles>, 2> String_To_Node_Role_Pairs{{
		{ "Peer", NodeRoles::Peer },
		{ "Api", NodeRoles::Api }
	}};

	bool TryParseValue(const std::string& str, NodeRoles& roles) {
		std::unordered_set<std::string> roleParts;
		if (!utils::TryParseValue(str, roleParts))
			return false;

		uint32_t rawRoles = 0u;
		for (const auto& rolePart : roleParts) {
			NodeRoles role;
			if (!utils::TryParseEnumValue(String_To_Node_Role_Pairs, rolePart, role))
				return false;

			rawRoles |= utils::to_underlying_type(role);
		}

		roles = static_cast<NodeRoles>(rawRoles);
		return true;
	}
}}
