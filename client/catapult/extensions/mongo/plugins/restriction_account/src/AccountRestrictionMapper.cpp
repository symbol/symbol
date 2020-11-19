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

#include "AccountRestrictionMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/restriction_account/src/model/AccountAddressRestrictionTransaction.h"
#include "plugins/txes/restriction_account/src/model/AccountMosaicRestrictionTransaction.h"
#include "plugins/txes/restriction_account/src/model/AccountOperationRestrictionTransaction.h"
#include "plugins/txes/restriction_account/src/model/AccountRestrictionFlags.h"
#include "plugins/txes/restriction_account/src/state/AccountRestrictionUtils.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		template<typename TRestrictionValue>
		void StreamValues(bson_stream::document& builder, const std::string& name, const TRestrictionValue* pValues, uint8_t numValues) {
			auto valuesArray = builder << name << bson_stream::open_array;
			for (auto i = 0u; i < numValues; ++i) {
				auto valueBuffer = state::ToVector(pValues[i]);
				valuesArray << ToBinary(valueBuffer.data(), valueBuffer.size());
			}

			valuesArray << bson_stream::close_array;
		}

		template<typename TRestrictionValue>
		struct AccountRestrictionTransactionStreamer {
			template<typename TTransaction>
			static void Stream(bson_stream::document& builder, const TTransaction& transaction) {
				builder << "restrictionFlags" << static_cast<int32_t>(transaction.RestrictionFlags);
				StreamValues(
						builder,
						"restrictionAdditions",
						transaction.RestrictionAdditionsPtr(),
						transaction.RestrictionAdditionsCount);
				StreamValues(
						builder,
						"restrictionDeletions",
						transaction.RestrictionDeletionsPtr(),
						transaction.RestrictionDeletionsCount);
			}
		};
	}

	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(AccountAddressRestriction, AccountRestrictionTransactionStreamer<Address>::Stream)
	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(AccountMosaicRestriction, AccountRestrictionTransactionStreamer<MosaicId>::Stream)
	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(AccountOperationRestriction, AccountRestrictionTransactionStreamer<model::EntityType>::Stream)
}}}
