#include "MultisigEntryMapper.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel

	namespace {
		void StreamPublicKeys(bson_stream::document& builder, const std::string& keySetName, const utils::KeySet& keys) {
			auto keyArray = builder << keySetName << bson_stream::open_array;
			for (const auto& key : keys)
				keyArray << ToBinary(key);

			keyArray << bson_stream::close_array;
		}
	}

	bsoncxx::document::value ToDbModel(const state::MultisigEntry& entry) {
		bson_stream::document builder;
		auto doc = builder << "multisig" << bson_stream::open_document
				<< "account" << ToBinary(entry.key())
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
		void ReadKeySet(utils::KeySet& keySet, const bsoncxx::array::view& dbKeys) {
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

		auto minApproval = static_cast<uint8_t>(dbMultisigEntry["minApproval"].get_int32());
		auto minRemoval = static_cast<uint8_t>(dbMultisigEntry["minRemoval"].get_int32());
		entry.setMinApproval(minApproval);
		entry.setMinRemoval(minRemoval);

		ReadKeySet(entry.cosignatories(), dbMultisigEntry["cosignatories"].get_array().value);
		ReadKeySet(entry.multisigAccounts(), dbMultisigEntry["multisigAccounts"].get_array().value);

		return entry;
	}

	// endregion
}}}
