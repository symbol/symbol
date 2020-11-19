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

#include "CoreMongo.h"
#include "MongoPluginManager.h"
#include "mappers/KeyLinkTransactionMapper.h"
#include "storages/MongoAccountStateCacheStorage.h"
#include "mongo/src/MongoReceiptPluginFactory.h"
#include "catapult/model/BlockChainConfiguration.h"

namespace catapult { namespace mongo {

	void RegisterCoreMongoSystem(MongoPluginManager& manager) {
		// transaction support
		manager.addTransactionSupport(mappers::CreateVotingKeyLinkTransactionMongoPlugin());
		manager.addTransactionSupport(mappers::CreateVrfKeyLinkTransactionMongoPlugin());

		// cache storage support
		manager.addStorageSupport(storages::CreateMongoAccountStateCacheStorage(manager.mongoContext(), manager.networkIdentifier()));

		// receipt support
		manager.addReceiptSupport(CreateBalanceChangeReceiptMongoPlugin(model::Receipt_Type_Harvest_Fee));
		manager.addReceiptSupport(CreateInflationReceiptMongoPlugin(model::Receipt_Type_Inflation));
	}
}}
