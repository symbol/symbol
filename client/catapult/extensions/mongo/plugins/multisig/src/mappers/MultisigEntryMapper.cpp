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

#include "MultisigEntryMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/utils/Casting.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	// region ToDbModel

	namespace {
		void StreamPublicKeys(bson_stream::document& builder, const std::string& keySetName, const utils::SortedKeySet& keys) {
			auto keyArray = builder << keySetName << bson_stream::open_array;
			for (const auto& key : keys)
				keyArray << ToBinary(key);

			keyArray << bson_stream::close_array;
		}
	}

	bsoncxx::document::value ToDbModel(const state::MultisigEntry& entry, const Address& accountAddress) {
		bson_stream::document builder;
		auto doc = builder << "multisig" << bson_stream::open_document
				<< "account" << ToBinary(entry.key())
				<< "accountAddress" << ToBinary(accountAddress)
				<< "minApproval" << static_cast<int32_t>(entry.minApproval())
				<< "minRemoval" << static_cast<int32_t>(entry.minRemoval());

		StreamPublicKeys(builder, "cosignatories", entry.cosignatories());
		StreamPublicKeys(builder, "multisigAccounts", entry.multisigAccounts());

		return doc
				<< bson_stream::close_document
				<< bson_stream::finalize;
	}

	// endregion

	// region ToModel

	namespace {
		void ReadKeySet(utils::SortedKeySet& keySet, const bsoncxx::array::view& dbKeys) {
			for (const auto& dbKey : dbKeys) {
				Key key;
				DbBinaryToModelArray(key, dbKey.get_binary());
				keySet.emplace(key);
			}
		}
	}

	state::MultisigEntry ToMultisigEntry(const bsoncxx::document::view& document) {
		auto dbMultisigEntry = document["multisig"];
		Key account;
		DbBinaryToModelArray(account, dbMultisigEntry["account"].get_binary());
		state::MultisigEntry entry(account);

		auto minApproval = ToUint8(dbMultisigEntry["minApproval"].get_int32());
		auto minRemoval = ToUint8(dbMultisigEntry["minRemoval"].get_int32());
		entry.setMinApproval(minApproval);
		entry.setMinRemoval(minRemoval);

		ReadKeySet(entry.cosignatories(), dbMultisigEntry["cosignatories"].get_array().value);
		ReadKeySet(entry.multisigAccounts(), dbMultisigEntry["multisigAccounts"].get_array().value);

		return entry;
	}

	// endregion
}}}
