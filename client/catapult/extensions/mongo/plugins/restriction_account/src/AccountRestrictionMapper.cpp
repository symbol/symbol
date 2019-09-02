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

#include "AccountRestrictionMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/restriction_account/src/model/AccountAddressRestrictionTransaction.h"
#include "plugins/txes/restriction_account/src/model/AccountMosaicRestrictionTransaction.h"
#include "plugins/txes/restriction_account/src/model/AccountOperationRestrictionTransaction.h"
#include "plugins/txes/restriction_account/src/model/AccountRestrictionTypes.h"
#include "plugins/txes/restriction_account/src/state/AccountRestrictionUtils.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		void StreamModification(
				bson_stream::array_context& context,
				model::AccountRestrictionModificationAction action,
				const std::vector<uint8_t>& value) {
			context
					<< bson_stream::open_document
						<< "modificationAction" << utils::to_underlying_type(action)
						<< "value" << ToBinary(value.data(), value.size())
					<< bson_stream::close_document;
		}

		template<typename TRestrictionValue>
		void StreamModifications(
				bson_stream::document& builder,
				const model::AccountRestrictionModification<TRestrictionValue>* pModifications,
				size_t numModifications) {
			auto modificationsArray = builder << "modifications" << bson_stream::open_array;
			for (auto i = 0u; i < numModifications; ++i)
				StreamModification(modificationsArray, pModifications[i].ModificationAction, state::ToVector(pModifications[i].Value));

			modificationsArray << bson_stream::close_array;
		}

		template<typename TRestrictionValue>
		struct AccountRestrictionTransactionStreamer {
			template<typename TTransaction>
			static void Stream(bson_stream::document& builder, const TTransaction& transaction) {
				builder << "restrictionType" << utils::to_underlying_type(transaction.RestrictionType);
				StreamModifications(builder, transaction.ModificationsPtr(), transaction.ModificationsCount);
			}
		};
	}

	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(AccountAddressRestriction, AccountRestrictionTransactionStreamer<Address>::Stream)
	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(AccountMosaicRestriction, AccountRestrictionTransactionStreamer<MosaicId>::Stream)
	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(AccountOperationRestriction, AccountRestrictionTransactionStreamer<model::EntityType>::Stream)
}}}
