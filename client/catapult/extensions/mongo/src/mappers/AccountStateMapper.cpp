#include "AccountStateMapper.h"
#include "MapperUtils.h"
#include "catapult/state/AccountState.h"

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel

	namespace {
		auto& StreamAccountMetadata(bson_stream::document& builder) {
			builder << "meta"
					<< bson_stream::open_document
					<< bson_stream::close_document;
			return builder;
		}

		auto& StreamAccountBalances(bson_stream::document& builder, const state::AccountBalances& balances) {
			auto mosaicsArray = builder << "mosaics" << bson_stream::open_array;
			for (const auto& entry : balances)
				StreamMosaic(mosaicsArray, entry.first, entry.second);

			mosaicsArray << bson_stream::close_array;
			return builder;
		}

		auto& StreamImportanceSnapshot(
				bson_stream::array_context& context,
				const state::AccountImportance::ImportanceSnapshot& importanceSnapshot) {
			context << bson_stream::open_document
						<< "value" << ToInt64(importanceSnapshot.Importance)
						<< "height" << ToInt64(importanceSnapshot.Height)
					<< bson_stream::close_document;
			return context;
		}

		auto& StreamAccountImportances(bson_stream::document& builder, const state::AccountImportance& importances) {
			// note: storing in db in reverse order, cause when loading, importanceInfo.set() allows only increasing heights
			std::array<state::AccountImportance::ImportanceSnapshot, Importance_History_Size> reverseSnapshots;
			auto index = 0u;
			for (const auto& snapshot : importances)
				reverseSnapshots[Importance_History_Size - index++ - 1] = snapshot;

			auto importancesArray = builder << "importances" << bson_stream::open_array;
			for (const auto& snapshot : reverseSnapshots) {
				if (model::ImportanceHeight(0) == snapshot.Height)
					continue;

				StreamImportanceSnapshot(importancesArray, snapshot);
			}

			importancesArray << bson_stream::close_array;
			return builder;
		}

		Key GetPublicKey(const state::AccountState& accountState) {
			return Height(0) == accountState.PublicKeyHeight ? Key{} : accountState.PublicKey;
		}
	}

	bsoncxx::document::value ToDbModel(const state::AccountState& accountState) {
		// account metadata
		bson_stream::document builder;
		StreamAccountMetadata(builder);

		// account data
		builder << "account" << bson_stream::open_document
				<< "address" << ToBinary(accountState.Address)
				<< "addressHeight" << ToInt64(accountState.AddressHeight)
				<< "publicKey" << ToBinary(GetPublicKey(accountState))
				<< "publicKeyHeight" << ToInt64(accountState.PublicKeyHeight);
		StreamAccountImportances(builder, accountState.ImportanceInfo);
		StreamAccountBalances(builder, accountState.Balances);
		builder << bson_stream::close_document;
		return builder << bson_stream::finalize;
	}

	// endregion

	// region ToAccountState

	namespace {
		void ToAccountImportance(state::AccountImportance& accountImportance, const bsoncxx::document::view& importanceDocument) {
			accountImportance.set(
					GetValue64<Importance>(importanceDocument["value"]),
					GetValue64<model::ImportanceHeight>(importanceDocument["height"]));
		}

		void ToAccountBalance(state::AccountBalances& accountBalances, const bsoncxx::document::view& mosaicDocument) {
			accountBalances.credit(GetValue64<MosaicId>(mosaicDocument["id"]), GetValue64<Amount>(mosaicDocument["amount"]));
		}
	}

	void ToAccountState(const bsoncxx::document::view& document, const AccountStateFactory& accountStateFactory) {
		auto accountDocument = document["account"];
		Address accountAddress;
		DbBinaryToModelArray(accountAddress, accountDocument["address"].get_binary());
		auto accountAddressHeight = GetValue64<Height>(accountDocument["addressHeight"]);

		auto& accountState = accountStateFactory(accountAddress, accountAddressHeight);
		DbBinaryToModelArray(accountState.PublicKey, accountDocument["publicKey"].get_binary());
		accountState.PublicKeyHeight = GetValue64<Height>(accountDocument["publicKeyHeight"]);

		auto dbImportances = accountDocument["importances"].get_array().value;
		for (const auto& importanceEntry : dbImportances)
			ToAccountImportance(accountState.ImportanceInfo, importanceEntry.get_document().view());

		auto dbMosaics = accountDocument["mosaics"].get_array().value;
		for (const auto& mosaicEntry : dbMosaics)
			ToAccountBalance(accountState.Balances, mosaicEntry.get_document().view());
	}

	// endregion
}}}
