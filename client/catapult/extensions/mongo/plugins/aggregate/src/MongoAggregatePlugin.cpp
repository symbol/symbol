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

#include "AggregateMapper.h"
#include "mongo/src/MongoPluginManager.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"

extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager) {
	using namespace catapult::model;
	using catapult::mongo::plugins::CreateAggregateTransactionMongoPlugin;

	manager.addTransactionSupport(CreateAggregateTransactionMongoPlugin(manager.transactionRegistry(), Entity_Type_Aggregate_Complete));
	manager.addTransactionSupport(CreateAggregateTransactionMongoPlugin(manager.transactionRegistry(), Entity_Type_Aggregate_Bonded));
}
