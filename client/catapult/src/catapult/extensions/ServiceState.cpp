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

#include "ServiceState.h"
#include "PeersConnectionTasks.h"
#include "catapult/io/BlockStorageCache.h"

namespace catapult { namespace extensions {

	supplier<model::HeightHashPair> CreateLocalFinalizedHeightHashPairSupplier(const ServiceState& state) {
		auto maxRollbackBlocks = state.config().BlockChain.MaxRollbackBlocks;
		if (0 == maxRollbackBlocks)
			return state.hooks().localFinalizedHeightHashPairSupplier();

		return [&storage = state.storage(), maxRollbackBlocks]() {
			auto storageView = storage.view();
			auto chainHeight = storageView.chainHeight();
			auto finalizedHeight = chainHeight.unwrap() <= maxRollbackBlocks
					? Height(1)
					: Height(chainHeight.unwrap() - maxRollbackBlocks);
			return model::HeightHashPair{ finalizedHeight, storageView.loadBlockElement(finalizedHeight)->EntityHash };
		};
	}

	supplier<Height> CreateLocalFinalizedHeightSupplier(const ServiceState& state) {
		auto heightHashPairSupplier = CreateLocalFinalizedHeightHashPairSupplier(state);
		return [heightHashPairSupplier]() {
			return heightHashPairSupplier().Height;
		};
	}

	SelectorSettings CreateOutgoingSelectorSettings(
			const ServiceState& state,
			ionet::ServiceIdentifier serviceId,
			ionet::NodeRoles requiredRole) {
		return SelectorSettings(
				state.cache(),
				state.config().BlockChain.TotalChainImportance,
				state.nodes(),
				serviceId,
				MapNodeRolesToIpProtocols(state.config().Node.Local.Roles),
				requiredRole,
				state.config().Node.OutgoingConnections);
	}

	SelectorSettings CreateIncomingSelectorSettings(const ServiceState& state, ionet::ServiceIdentifier serviceId) {
		return SelectorSettings(
			state.cache(),
			state.config().BlockChain.TotalChainImportance,
			state.nodes(),
			serviceId,
			state.config().Node.IncomingConnections);
	}
}}
