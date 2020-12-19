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

#include "AccountStateMapper.h"
#include "MapperUtils.h"
#include "catapult/state/AccountState.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel

	namespace {
		using PublicKeyType = state::AccountPublicKeys::KeyType;
		using bson_subdocument = bsoncxx::v_noabi::builder::stream::key_context<
			bsoncxx::v_noabi::builder::stream::key_context<bsoncxx::v_noabi::builder::stream::closed_context>
		>;

		void StreamPublicKey(
				bson_subdocument& builder,
				const std::string& name,
				PublicKeyType mask,
				PublicKeyType keyType,
				const state::AccountPublicKeys::PublicKeyAccessor<Key>& publicKeyAccessor) {
			if (!HasFlag(keyType, mask))
				return;

			builder
					<< name << bson_stream::open_document
						<< "publicKey" << ToBinary(publicKeyAccessor.get())
					<< bson_stream::close_document;
		}

		void StreamPublicKeys(
				bson_subdocument& builder,
				const std::string& name,
				const state::AccountPublicKeys::PublicKeysAccessor<model::PinnedVotingKey>& publicKeysAccessor) {
			if (0 == publicKeysAccessor.size())
				return;

			builder << name << bson_stream::open_document;
			auto publicKeysArray = builder << "publicKeys" << bson_stream::open_array;

			for (auto i = 0u; i < publicKeysAccessor.size(); ++i) {
				const auto& pinnedPublicKey = publicKeysAccessor.get(i);
				publicKeysArray
						<< bson_stream::open_document
							<< "publicKey" << ToBinary(pinnedPublicKey.VotingKey)
							<< "startEpoch" << ToInt32(pinnedPublicKey.StartEpoch)
							<< "endEpoch" << ToInt32(pinnedPublicKey.EndEpoch)
						<< bson_stream::close_document;
			}

			publicKeysArray << bson_stream::close_array;
			builder << bson_stream::close_document;
		}

		void StreamAccountPublicKeys(bson_stream::document& builder, const state::AccountPublicKeys& accountPublicKeys) {
			auto publicKeysDocument = builder << "supplementalPublicKeys" << bson_stream::open_document;

			auto mask = accountPublicKeys.mask();
			StreamPublicKey(publicKeysDocument, "linked", mask, PublicKeyType::Linked, accountPublicKeys.linked());
			StreamPublicKey(publicKeysDocument, "node", mask, PublicKeyType::Node, accountPublicKeys.node());
			StreamPublicKey(publicKeysDocument, "vrf", mask, PublicKeyType::VRF, accountPublicKeys.vrf());
			StreamPublicKeys(publicKeysDocument, "voting", accountPublicKeys.voting());

			publicKeysDocument << bson_stream::close_document;
		}

		void StreamAccountImportanceInformation(
				bson_stream::document& builder,
				const state::AccountImportanceSnapshots& snapshots,
				const state::AccountActivityBuckets& buckets) {
			auto importancesArray = builder << "importances" << bson_stream::open_array;

			if (snapshots.active()) {
				auto i = 0u;
				for (const auto& snapshot : snapshots) {
					if (i++ >= Importance_History_Size - Rollback_Buffer_Size)
						break;

					importancesArray
							<< bson_stream::open_document
								<< "value" << ToInt64(snapshot.Importance)
								<< "height" << ToInt64(snapshot.Height)
							<< bson_stream::close_document;
				}
			}

			importancesArray << bson_stream::close_array;

			auto activityBucketsArray = builder << "activityBuckets" << bson_stream::open_array;

			if (snapshots.active()) {
				auto i = 0u;
				for (const auto& bucket : buckets) {
					if (i++ >= Activity_Bucket_History_Size - Rollback_Buffer_Size)
						break;

					activityBucketsArray
							<< bson_stream::open_document
								<< "startHeight" << ToInt64(bucket.StartHeight)
								<< "totalFeesPaid" << ToInt64(bucket.TotalFeesPaid)
								<< "beneficiaryCount" << static_cast<int32_t>(bucket.BeneficiaryCount)
								<< "rawScore" << static_cast<int64_t>(bucket.RawScore)
							<< bson_stream::close_document;
				}
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
					<< "version" << 1
					<< "address" << ToBinary(accountState.Address)
					<< "addressHeight" << ToInt64(accountState.AddressHeight)
					<< "publicKey" << ToBinary(accountState.PublicKey)
					<< "publicKeyHeight" << ToInt64(accountState.PublicKeyHeight)
					<< "accountType" << utils::to_underlying_type(accountState.AccountType);

		StreamAccountPublicKeys(builder, accountState.SupplementalPublicKeys);
		StreamAccountImportanceInformation(builder, accountState.ImportanceSnapshots, accountState.ActivityBuckets);
		StreamAccountBalances(builder, accountState.Balances);
		builder << bson_stream::close_document;
		return builder << bson_stream::finalize;
	}

	// endregion
}}}
