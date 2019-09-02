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

#include "TransactionRegistryFactory.h"
#include "catapult/plugins/MosaicAliasTransactionPlugin.h"
#include "catapult/plugins/MosaicDefinitionTransactionPlugin.h"
#include "catapult/plugins/MosaicSupplyChangeTransactionPlugin.h"
#include "catapult/plugins/NamespaceRegistrationTransactionPlugin.h"
#include "catapult/plugins/TransferTransactionPlugin.h"

namespace catapult { namespace tools { namespace nemgen {

	model::TransactionRegistry CreateTransactionRegistry() {
		plugins::MosaicRentalFeeConfiguration mosaicConfig;
		plugins::NamespaceRentalFeeConfiguration namespaceConfig;
		model::TransactionRegistry registry;
		registry.registerPlugin(plugins::CreateMosaicAliasTransactionPlugin());
		registry.registerPlugin(plugins::CreateMosaicDefinitionTransactionPlugin(mosaicConfig));
		registry.registerPlugin(plugins::CreateMosaicSupplyChangeTransactionPlugin());
		registry.registerPlugin(plugins::CreateNamespaceRegistrationTransactionPlugin(namespaceConfig));
		registry.registerPlugin(plugins::CreateTransferTransactionPlugin());
		return registry;
	}
}}}
