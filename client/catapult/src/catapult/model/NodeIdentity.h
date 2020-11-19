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
#include "catapult/types.h"
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace model {

	/// Unique node identifier.
	struct NodeIdentity {
	public:
		/// Identity key.
		Key PublicKey;

		/// Host (must be resolved).
		std::string Host;

	public:
		/// Insertion operator for outputting \a identity to \a out.
		friend std::ostream& operator<<(std::ostream& out, const NodeIdentity& identity);
	};

	/// Strategy to use for comparing NodeIdentity structures.
	enum class NodeIdentityEqualityStrategy {
		/// Identities are uniquely identified by identity key.
		Key,

		/// Identities are uniquely identified by host.
		Host,

		/// Identities are uniquely identified by key and host.
		Key_And_Host
	};

	/// Tries to parse \a strategyName into a node identity equality \a strategy.
	bool TryParseValue(const std::string& strategyName, NodeIdentityEqualityStrategy& strategy);

	/// Equality object for NodeIdentity.
	class NodeIdentityEquality {
	public:
		/// Creates an equality object with \a strategy.
		explicit NodeIdentityEquality(NodeIdentityEqualityStrategy strategy);

	public:
		/// Returns \c true if \a lhs and \a rhs are equal.
		bool operator()(const NodeIdentity& lhs, const NodeIdentity& rhs) const;

	private:
		NodeIdentityEqualityStrategy m_strategy;
	};

	/// Hasher object for NodeIdentity.
	class NodeIdentityHasher {
	public:
		/// Creates a hasher object with \a strategy.
		explicit NodeIdentityHasher(NodeIdentityEqualityStrategy strategy);

	public:
		/// Hashes \a identity.
		size_t operator()(const NodeIdentity& identity) const;

	private:
		NodeIdentityEqualityStrategy m_strategy;
	};

	/// Unordered set of node identities.
	using NodeIdentitySet = std::unordered_set<NodeIdentity, NodeIdentityHasher, NodeIdentityEquality>;

	/// Creates an empty node identity set with \a strategy.
	NodeIdentitySet CreateNodeIdentitySet(NodeIdentityEqualityStrategy strategy);

	/// Map of node identities to associated data.
	template<typename TValue>
	using NodeIdentityMap = std::unordered_map<NodeIdentity, TValue, NodeIdentityHasher, NodeIdentityEquality>;

	/// Creates an empty node identity map with \a strategy.
	template<typename TValue>
	NodeIdentityMap<TValue> CreateNodeIdentityMap(NodeIdentityEqualityStrategy strategy) {
		return NodeIdentityMap<TValue>(0, NodeIdentityHasher(strategy), NodeIdentityEquality(strategy));
	}
}}
