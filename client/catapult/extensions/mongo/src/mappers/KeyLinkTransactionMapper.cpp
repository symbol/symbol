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

#include "KeyLinkTransactionMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/coresystem/src/model/VotingKeyLinkTransaction.h"
#include "plugins/coresystem/src/model/VrfKeyLinkTransaction.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		template<typename TTransaction>
		void StreamVotingTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder
					<< "linkedPublicKey" << ToBinary(transaction.LinkedPublicKey)
					<< "startPoint" << ToInt64(transaction.StartPoint)
					<< "endPoint" << ToInt64(transaction.EndPoint)
					<< "linkAction" << utils::to_underlying_type(transaction.LinkAction);
		}

		template<typename TTransaction>
		void StreamVrfTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder
					<< "linkedPublicKey" << ToBinary(transaction.LinkedPublicKey)
					<< "linkAction" << utils::to_underlying_type(transaction.LinkAction);
		}
	}

	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(VotingKeyLink, StreamVotingTransaction)
	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(VrfKeyLink, StreamVrfTransaction)
}}}
