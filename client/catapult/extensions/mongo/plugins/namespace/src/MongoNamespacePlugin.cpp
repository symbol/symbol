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

#include "AddressAliasMapper.h"
#include "MosaicAliasMapper.h"
#include "NamespaceRegistrationMapper.h"
#include "storages/MongoNamespaceCacheStorage.h"
#include "mongo/src/MongoPluginManager.h"
#include "mongo/src/MongoReceiptPluginFactory.h"
#include "plugins/txes/namespace/src/constants.h"
#include "plugins/txes/namespace/src/model/NamespaceReceiptType.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager) {
	using namespace catapult;

	// transaction support
	manager.addTransactionSupport(mongo::plugins::CreateAddressAliasTransactionMongoPlugin());
	manager.addTransactionSupport(mongo::plugins::CreateMosaicAliasTransactionMongoPlugin());
	manager.addTransactionSupport(mongo::plugins::CreateNamespaceRegistrationTransactionMongoPlugin());

	// cache storage support
	manager.addStorageSupport(mongo::plugins::CreateMongoNamespaceCacheStorage(manager.mongoContext(), manager.networkIdentifier()));

	// receipt support
	manager.addReceiptSupport(mongo::CreateArtifactExpiryReceiptMongoPlugin<NamespaceId>(model::Receipt_Type_Namespace_Expired));
	manager.addReceiptSupport(mongo::CreateArtifactExpiryReceiptMongoPlugin<NamespaceId>(model::Receipt_Type_Namespace_Deleted));
	manager.addReceiptSupport(mongo::CreateBalanceTransferReceiptMongoPlugin(model::Receipt_Type_Namespace_Rental_Fee));
}
