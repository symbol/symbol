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

#include "MosaicDefinitionMapper.h"
#include "MosaicSupplyChangeMapper.h"
#include "storages/MongoMosaicCacheStorage.h"
#include "mongo/src/MongoPluginManager.h"
#include "mongo/src/MongoReceiptPluginFactory.h"
#include "plugins/txes/mosaic/src/model/MosaicReceiptType.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager) {
	using namespace catapult;

	// transaction support
	manager.addTransactionSupport(mongo::plugins::CreateMosaicDefinitionTransactionMongoPlugin());
	manager.addTransactionSupport(mongo::plugins::CreateMosaicSupplyChangeTransactionMongoPlugin());

	// cache storage support
	manager.addStorageSupport(mongo::plugins::CreateMongoMosaicCacheStorage(manager.mongoContext(), manager.networkIdentifier()));

	// receipt support
	manager.addReceiptSupport(mongo::CreateArtifactExpiryReceiptMongoPlugin<MosaicId>(model::Receipt_Type_Mosaic_Expired));
	manager.addReceiptSupport(mongo::CreateBalanceTransferReceiptMongoPlugin(model::Receipt_Type_Mosaic_Rental_Fee));
}
