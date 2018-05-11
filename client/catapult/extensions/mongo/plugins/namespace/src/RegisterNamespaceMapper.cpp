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

#include "RegisterNamespaceMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/RegisterNamespaceTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			if (0 == transaction.NamespaceNameSize)
				CATAPULT_THROW_RUNTIME_ERROR("cannot map register namespace transaction without name");

			builder << "namespaceType" << utils::to_underlying_type(transaction.NamespaceType);

			if (transaction.IsRootRegistration())
				builder << "duration" << ToInt64(transaction.Duration);
			else
				builder << "parentId" << ToInt64(transaction.ParentId);

			builder
					<< "namespaceId" << ToInt64(transaction.NamespaceId)
					<< "name" << ToBinary(transaction.NamePtr(), transaction.NamespaceNameSize);
		}
	}

	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(RegisterNamespace, StreamTransaction)
}}}
