/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "catapult/model/NodeIdentity.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Shared test utils for ConnectionContainer (PacketReaders and PacketWriters) tests.
	class ConnectionContainerTestUtils {
	public:
		/// Maps \a identityKey to a node identity.
		static model::NodeIdentity ToIdentity(const Key& identityKey) {
			return { identityKey, "11.22.33.44" };
		}

		/// Maps all \a publicKeys to 0-based node identities.
		static model::NodeIdentitySet PublicKeysToIdentitySet(const std::vector<Key>& publicKeys) {
			auto identities = CreateNodeIdentitySet(model::NodeIdentityEqualityStrategy::Key);

			auto i = 0u;
			for (const auto& publicKey : publicKeys)
				identities.insert({ publicKey, std::to_string(i++) });

			return identities;
		}

		/// Maps all \a hosts to node identities with specified \a identityKey.
		static model::NodeIdentitySet HostsToIdentitySet(const std::vector<std::string>& hosts, const Key& identityKey) {
			auto identities = CreateNodeIdentitySet(model::NodeIdentityEqualityStrategy::Host);

			for (const auto& host : hosts)
				identities.insert({ identityKey, host });

			return identities;
		}

		/// Maps all \a publicKeys at \a indexes to 0-based node identities.
		static model::NodeIdentitySet PickIdentities(const std::vector<Key>& publicKeys, std::initializer_list<size_t> indexes) {
			auto identities = CreateNodeIdentitySet(model::NodeIdentityEqualityStrategy::Key);

			for (auto index : indexes)
				identities.insert({ publicKeys[index], std::to_string(index) });

			return identities;
		}
	};
}}

