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

#include "TimeSynchronizationCacheTestUtils.h"
#include "catapult/cache_core/AccountStateCacheDelta.h"
#include "catapult/ionet/NodeContainer.h"
#include "tests/test/net/NodeTestUtils.h"

namespace catapult { namespace test {

	void AddAccount(
			cache::AccountStateCacheDelta& delta,
			const Key& publicKey,
			Importance importance,
			model::ImportanceHeight importanceHeight) {
		auto& accountState = delta.addAccount(publicKey, Height(100));
		accountState.ImportanceInfo.set(importance, importanceHeight);
		accountState.Balances.credit(Xem_Id, Amount(1000));
	}

	void AddNode(ionet::NodeContainer& container, const Key& identityKey, const std::string& nodeName) {
		auto modifier = container.modifier();
		auto metadata = ionet::NodeMetadata(model::NetworkIdentifier::Zero, nodeName);
		metadata.Roles = ionet::NodeRoles::Peer;
		ionet::Node node(identityKey, CreateLocalHostNodeEndpoint(), metadata);
		modifier.add(node, ionet::NodeSource::Dynamic);
		modifier.provisionConnectionState(ionet::ServiceIdentifier(0x53594E43), identityKey).Age = 5;
	}
}}
