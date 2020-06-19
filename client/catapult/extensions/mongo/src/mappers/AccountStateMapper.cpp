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

#include "AccountStateMapper.h"
#include "MapperUtils.h"
#include "catapult/state/AccountState.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel

	namespace {
		template<typename TAccountPublicKey>
		void StreamPublicKey(
				bson_stream::array_context& context,
				state::AccountPublicKeys::KeyType mask,
				state::AccountPublicKeys::KeyType keyType,
				const state::AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>& publicKeyAccessor) {
			if (!HasFlag(keyType, mask))
				return;

			context
					<< bson_stream::open_document
						<< "keyType" << utils::to_underlying_type(keyType)
						<< "key" << ToBinary(publicKeyAccessor.get())
					<< bson_stream::close_document;
		}

		template<>
		void StreamPublicKey(
				bson_stream::array_context& context,
				state::AccountPublicKeys::KeyType mask,
				state::AccountPublicKeys::KeyType keyType,
				const state::AccountPublicKeys::PublicKeyAccessor<PinnedVotingKey>& publicKeyAccessor) {
			if (!HasFlag(keyType, mask))
				return;

			context
					<< bson_stream::open_document
						<< "keyType" << utils::to_underlying_type(keyType)
						<< "key" << ToBinary(publicKeyAccessor.get().VotingKey)
						<< "startPoint" << ToInt64(publicKeyAccessor.get().StartPoint)
						<< "endPoint" << ToInt64(publicKeyAccessor.get().EndPoint)
					<< bson_stream::close_document;
		}

		void StreamAccountPublicKeys(bson_stream::document& builder, const state::AccountPublicKeys& accountPublicKeys) {
			auto publicKeysArray = builder << "supplementalPublicKeys" << bson_stream::open_array;

			auto mask = accountPublicKeys.mask();
			StreamPublicKey(publicKeysArray, mask, state::AccountPublicKeys::KeyType::Linked, accountPublicKeys.linked());
			StreamPublicKey(publicKeysArray, mask, state::AccountPublicKeys::KeyType::Node, accountPublicKeys.node());
			StreamPublicKey(publicKeysArray, mask, state::AccountPublicKeys::KeyType::VRF, accountPublicKeys.vrf());
			StreamPublicKey(publicKeysArray, mask, state::AccountPublicKeys::KeyType::Voting, accountPublicKeys.voting());

			publicKeysArray << bson_stream::close_array;
		}

		void StreamAccountImportanceSnapshots(bson_stream::document& builder, const state::AccountImportanceSnapshots& snapshots) {
			auto importancesArray = builder << "importances" << bson_stream::open_array;
			for (const auto& snapshot : snapshots) {
				if (model::ImportanceHeight(0) == snapshot.Height)
					break;

				importancesArray
						<< bson_stream::open_document
							<< "value" << ToInt64(snapshot.Importance)
							<< "height" << ToInt64(snapshot.Height)
						<< bson_stream::close_document;
			}

			importancesArray << bson_stream::close_array;
		}

		void StreamAccountActivityBuckets(bson_stream::document& builder, const state::AccountActivityBuckets& buckets) {
			auto activityBucketsArray = builder << "activityBuckets" << bson_stream::open_array;
			for (const auto& bucket : buckets) {
				if (model::ImportanceHeight(0) == bucket.StartHeight)
					break;

				activityBucketsArray
						<< bson_stream::open_document
							<< "startHeight" << ToInt64(bucket.StartHeight)
							<< "totalFeesPaid" << ToInt64(bucket.TotalFeesPaid)
							<< "beneficiaryCount" << static_cast<int32_t>(bucket.BeneficiaryCount)
							<< "rawScore" << static_cast<int64_t>(bucket.RawScore)
						<< bson_stream::close_document;
			}

			activityBucketsArray << bson_stream::close_array;
		}

		void StreamAccountBalances(bson_stream::document& builder, const state::AccountBalances& balances) {
			auto mosaicsArray = builder << "mosaics" << bson_stream::open_array;
			for (const auto& entry : balances)
				StreamMosaic(mosaicsArray, entry.first, entry.second);

			mosaicsArray << bson_stream::close_array;
		}
	}

	bsoncxx::document::value ToDbModel(const state::AccountState& accountState) {
		bson_stream::document builder;
		builder
				<< "account" << bson_stream::open_document
					<< "address" << ToBinary(accountState.Address)
					<< "addressHeight" << ToInt64(accountState.AddressHeight)
					<< "publicKey" << ToBinary(accountState.PublicKey)
					<< "publicKeyHeight" << ToInt64(accountState.PublicKeyHeight)
					<< "accountType" << utils::to_underlying_type(accountState.AccountType);

		StreamAccountPublicKeys(builder, accountState.SupplementalPublicKeys);
		StreamAccountImportanceSnapshots(builder, accountState.ImportanceSnapshots);
		StreamAccountActivityBuckets(builder, accountState.ActivityBuckets);
		StreamAccountBalances(builder, accountState.Balances);
		builder << bson_stream::close_document;
		return builder << bson_stream::finalize;
	}

	// endregion
}}}
