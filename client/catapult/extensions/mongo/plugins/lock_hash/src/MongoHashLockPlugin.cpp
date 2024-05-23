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

#include "HashLockMapper.h"
#include "mongo/src/MongoPluginManager.h"
#include "mongo/src/MongoReceiptPluginFactory.h"
#include "plugins/txes/lock_hash/src/model/HashLockReceiptType.h"
#include "storages/MongoHashLockInfoCacheStorage.h"

extern "C" PLUGIN_API void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager)
{
    // transaction support
    manager.addTransactionSupport(catapult::mongo::plugins::CreateHashLockTransactionMongoPlugin());

    // cache storage support
    manager.addStorageSupport(
        catapult::mongo::plugins::CreateMongoHashLockInfoCacheStorage(manager.mongoContext(), manager.networkIdentifier()));

    // receipt support
    manager.addReceiptSupport(catapult::mongo::CreateBalanceChangeReceiptMongoPlugin(catapult::model::Receipt_Type_LockHash_Created));
    manager.addReceiptSupport(catapult::mongo::CreateBalanceChangeReceiptMongoPlugin(catapult::model::Receipt_Type_LockHash_Completed));
    manager.addReceiptSupport(catapult::mongo::CreateBalanceChangeReceiptMongoPlugin(catapult::model::Receipt_Type_LockHash_Expired));
}
