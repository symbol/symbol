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

#include "NodeIdentity.h"
#include "catapult/utils/ConfigurationValueParsers.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace model {

	// region NodeIdentity

	std::ostream& operator<<(std::ostream& out, const NodeIdentity& identity) {
		out << identity.PublicKey;

		// host is resolved, so don't need to check for unprintable characters
		if (!identity.Host.empty())
			out << " @ " << identity.Host;

		return out;
	}

	// endregion

	// region NodeIdentityEqualityStrategy

	namespace {
		const std::array<std::pair<const char*, NodeIdentityEqualityStrategy>, 2> String_To_Node_Identity_Equality_Strategy_Pairs{{
			{ "public-key", NodeIdentityEqualityStrategy::Key },
			{ "host", NodeIdentityEqualityStrategy::Host }
		}};
	}

	bool TryParseValue(const std::string& strategyName, NodeIdentityEqualityStrategy& strategy) {
		return utils::TryParseEnumValue(String_To_Node_Identity_Equality_Strategy_Pairs, strategyName, strategy);
	}

	// endregion

	// region NodeIdentityEquality

	NodeIdentityEquality::NodeIdentityEquality(NodeIdentityEqualityStrategy strategy) : m_strategy(strategy)
	{}

	bool NodeIdentityEquality::operator()(const NodeIdentity& lhs, const NodeIdentity& rhs) const {
		switch (m_strategy) {
		case NodeIdentityEqualityStrategy::Key:
			return lhs.PublicKey == rhs.PublicKey;

		case NodeIdentityEqualityStrategy::Host:
			return lhs.Host == rhs.Host;

		default:
			return lhs.PublicKey == rhs.PublicKey && lhs.Host == rhs.Host;
		}
	}

	// endregion

	// region NodeIdentityHasher

	NodeIdentityHasher::NodeIdentityHasher(NodeIdentityEqualityStrategy strategy) : m_strategy(strategy)
	{}

	size_t NodeIdentityHasher::operator()(const NodeIdentity& identity) const {
		switch (m_strategy) {
		case NodeIdentityEqualityStrategy::Key:
			return utils::ArrayHasher<Key>()(identity.PublicKey);

		case NodeIdentityEqualityStrategy::Host:
			return std::hash<std::string>()(identity.Host);

		default:
			return utils::ArrayHasher<Key>()(identity.PublicKey) ^ std::hash<std::string>()(identity.Host);
		}
	}

	// endregion

	// region CreateNodeIdentitySet

	NodeIdentitySet CreateNodeIdentitySet(NodeIdentityEqualityStrategy strategy) {
		return NodeIdentitySet(0, NodeIdentityHasher(strategy), NodeIdentityEquality(strategy));
	}

	// endregion
}}
