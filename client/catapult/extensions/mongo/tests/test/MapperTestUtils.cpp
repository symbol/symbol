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

#include "MapperTestUtils.h"
#include "mongo/src/MongoTransactionMetadata.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/Block.h"
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/Cosignature.h"
#include "catapult/model/Receipt.h"
#include "catapult/state/AccountState.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"
#include <bsoncxx/types.hpp>

namespace catapult { namespace test {

	namespace {
		UnresolvedAddress ToUnresolvedAddress(const uint8_t* pByteArray) {
			UnresolvedAddress address;
			std::memcpy(address.data(), pByteArray, Address::Size);
			return address;
		}

		template<typename TEntity>
		void AssertEqualEntityData(const TEntity& entity, const bsoncxx::document::view& dbEntity) {
			EXPECT_EQ(entity.SignerPublicKey, GetKeyValue(dbEntity, "signerPublicKey"));

			EXPECT_EQ(entity.Version, GetInt32(dbEntity, "version"));
			EXPECT_EQ(entity.Network, static_cast<model::NetworkIdentifier>(GetInt32(dbEntity, "network")));
			EXPECT_EQ(entity.Type, static_cast<model::EntityType>(GetInt32(dbEntity, "type")));
		}

		void AssertEqualHashArray(const std::vector<Hash256>& hashes, const bsoncxx::document::view& dbHashes) {
			ASSERT_EQ(hashes.size(), GetFieldCount(dbHashes));

			auto i = 0u;
			for (const auto& dbHash : dbHashes) {
				auto hash = GetByteArrayFromMongoSource<Hash256>(dbHash);
				EXPECT_EQ(hashes[i], hash);
				++i;
			}
		}
	}

	void AssertEqualEmbeddedTransactionData(const model::EmbeddedTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
		AssertEqualEntityData(transaction, dbTransaction);
	}

	void AssertEqualVerifiableEntityData(const model::VerifiableEntity& entity, const bsoncxx::document::view& dbEntity) {
		EXPECT_EQ(entity.Size, GetUint32(dbEntity, "size"));
		EXPECT_EQ(entity.Signature, GetSignatureValue(dbEntity, "signature"));
		AssertEqualEntityData(entity, dbEntity);
	}

	void AssertEqualTransactionData(const model::Transaction& transaction, const bsoncxx::document::view& dbTransaction) {
		AssertEqualVerifiableEntityData(transaction, dbTransaction);
		EXPECT_EQ(transaction.MaxFee, Amount(GetUint64(dbTransaction, "maxFee")));
		EXPECT_EQ(transaction.Deadline, Timestamp(GetUint64(dbTransaction, "deadline")));
	}

	void AssertEqualTransactionMetadata(
			const mongo::MongoTransactionMetadata& metadata,
			const bsoncxx::document::view& dbTransactionMetadata) {
		EXPECT_EQ(metadata.EntityHash, GetHashValue(dbTransactionMetadata, "hash"));
		EXPECT_EQ(metadata.MerkleComponentHash, GetHashValue(dbTransactionMetadata, "merkleComponentHash"));
		auto dbAddresses = dbTransactionMetadata["addresses"].get_array().value;
		model::UnresolvedAddressSet addresses;
		for (const auto& dbAddress : dbAddresses) {
			ASSERT_EQ(Address::Size, dbAddress.get_binary().size);
			addresses.insert(ToUnresolvedAddress(dbAddress.get_binary().bytes));
		}

		EXPECT_EQ(metadata.Addresses.size(), addresses.size());
		EXPECT_EQ(metadata.Addresses, addresses);
		EXPECT_EQ(metadata.Height, Height(GetUint64(dbTransactionMetadata, "height")));
		EXPECT_EQ(metadata.Index, GetUint32(dbTransactionMetadata, "index"));
	}

	void AssertEqualBlockData(const model::Block& block, const bsoncxx::document::view& dbBlock) {
		// - 6 fields from VerifiableEntity, 12 fields from Block, 4 fields from ImportanceBlockFooter
		EXPECT_EQ(18u + (model::IsImportanceBlock(block.Type, block.Version) ? 4 : 0), GetFieldCount(dbBlock));
		AssertEqualVerifiableEntityData(block, dbBlock);

		EXPECT_EQ(block.Height, Height(GetUint64(dbBlock, "height")));
		EXPECT_EQ(block.Timestamp, Timestamp(GetUint64(dbBlock, "timestamp")));
		EXPECT_EQ(block.Difficulty, Difficulty(GetUint64(dbBlock, "difficulty")));
		EXPECT_EQ(block.GenerationHashProof.Gamma, GetBinaryArray<crypto::ProofGamma::Size>(dbBlock, "proofGamma"));
		EXPECT_EQ(
				block.GenerationHashProof.VerificationHash,
				GetBinaryArray<crypto::ProofVerificationHash::Size>(dbBlock, "proofVerificationHash"));
		EXPECT_EQ(block.GenerationHashProof.Scalar, GetBinaryArray<crypto::ProofScalar::Size>(dbBlock, "proofScalar"));
		EXPECT_EQ(block.PreviousBlockHash, GetHashValue(dbBlock, "previousBlockHash"));
		EXPECT_EQ(block.TransactionsHash, GetHashValue(dbBlock, "transactionsHash"));
		EXPECT_EQ(block.ReceiptsHash, GetHashValue(dbBlock, "receiptsHash"));
		EXPECT_EQ(block.StateHash, GetHashValue(dbBlock, "stateHash"));
		EXPECT_EQ(block.BeneficiaryAddress, GetAddressValue(dbBlock, "beneficiaryAddress"));
		EXPECT_EQ(block.FeeMultiplier, BlockFeeMultiplier(GetUint32(dbBlock, "feeMultiplier")));

		if (model::IsImportanceBlock(block.Type, block.Version)) {
			const auto& blockFooter = model::GetBlockFooter<model::ImportanceBlockFooter>(block);
			EXPECT_EQ(blockFooter.VotingEligibleAccountsCount, GetUint32(dbBlock, "votingEligibleAccountsCount"));
			EXPECT_EQ(blockFooter.HarvestingEligibleAccountsCount, GetUint64(dbBlock, "harvestingEligibleAccountsCount"));
			EXPECT_EQ(blockFooter.TotalVotingBalance, Amount(GetUint64(dbBlock, "totalVotingBalance")));
			EXPECT_EQ(blockFooter.PreviousImportanceBlockHash, GetHashValue(dbBlock, "previousImportanceBlockHash"));
		}
	}

	void AssertEqualBlockMetadata(
			const model::BlockElement& blockElement,
			Amount totalFee,
			const BlockMetadataCounts& blockMetadataCounts,
			const std::vector<Hash256>& transactionMerkleTree,
			const std::vector<Hash256>& statementMerkleTree,
			const bsoncxx::document::view& dbBlockMetadata) {
		auto expectedFieldCount = statementMerkleTree.empty() ? 7u : 9u;
		EXPECT_EQ(expectedFieldCount, GetFieldCount(dbBlockMetadata));
		EXPECT_EQ(blockElement.EntityHash, GetHashValue(dbBlockMetadata, "hash"));
		EXPECT_EQ(blockElement.GenerationHash, GetGenerationHashValue(dbBlockMetadata, "generationHash"));
		EXPECT_EQ(totalFee, Amount(GetUint64(dbBlockMetadata, "totalFee")));
		EXPECT_EQ(blockMetadataCounts.TransactionsCount, GetUint32(dbBlockMetadata, "transactionsCount"));
		EXPECT_EQ(blockMetadataCounts.TotalTransactionsCount, GetUint32(dbBlockMetadata, "totalTransactionsCount"));

		AssertEqualHashArray(blockElement.SubCacheMerkleRoots, dbBlockMetadata["stateHashSubCacheMerkleRoots"].get_array().value);
		AssertEqualHashArray(transactionMerkleTree, dbBlockMetadata["transactionMerkleTree"].get_array().value);
		if (!statementMerkleTree.empty()) {
			EXPECT_EQ(blockMetadataCounts.StatementsCount, GetUint32(dbBlockMetadata, "statementsCount"));
			AssertEqualHashArray(statementMerkleTree, dbBlockMetadata["statementMerkleTree"].get_array().value);
		}
	}

	namespace {
		using PublicKeyType = state::AccountPublicKeys::KeyType;

		void Advance(PublicKeyType& keyType) {
			keyType = static_cast<PublicKeyType>(utils::to_underlying_type(keyType) << 1);
		}

		void AssertPublicKeySubDocument(
				const bsoncxx::document::view& dbAccountPublicKeys,
				const std::string& name,
				const state::AccountPublicKeys::PublicKeyAccessor<Key>& expectedPublicKeyAccessor) {
			auto dbPublicKeyDocument = dbAccountPublicKeys[name].get_document();
			EXPECT_EQ(1u, GetFieldCount(dbPublicKeyDocument.view()));
			EXPECT_EQ(expectedPublicKeyAccessor.get(), GetKeyValue(dbPublicKeyDocument.view(), "publicKey"));
		}

		void AssertPublicKeysSubDocument(
				const bsoncxx::document::view& dbAccountPublicKeys,
				const std::string& name,
				const state::AccountPublicKeys::PublicKeysAccessor<model::PinnedVotingKey>& expectedPublicKeysAccessor) {
			auto dbPublicKeysDocument = dbAccountPublicKeys[name].get_document();
			EXPECT_EQ(1u, GetFieldCount(dbPublicKeysDocument.view()));

			auto i = 0u;
			for (const auto& dbPublicKeyDocument : dbPublicKeysDocument.view()["publicKeys"].get_array().value) {
				const auto& expectedPublicKey = expectedPublicKeysAccessor.get(i);
				EXPECT_EQ(expectedPublicKey.VotingKey, GetVotingKeyValue(dbPublicKeyDocument, "publicKey")) << "at " << i;
				EXPECT_EQ(expectedPublicKey.StartEpoch, FinalizationEpoch(GetUint32(dbPublicKeyDocument, "startEpoch"))) << "at " << i;
				EXPECT_EQ(expectedPublicKey.EndEpoch, FinalizationEpoch(GetUint32(dbPublicKeyDocument, "endEpoch"))) << "at " << i;
				++i;
			}

			EXPECT_EQ(i, expectedPublicKeysAccessor.size());
		}

		void AssertEqualAccountPublicKeys(
				const state::AccountPublicKeys& accountPublicKeys,
				const bsoncxx::document::view& dbAccountPublicKeys) {
			size_t numExpectedFields = 0u;
			for (auto keyType = PublicKeyType::Linked; keyType <= PublicKeyType::All; Advance(keyType)) {
				if (!HasFlag(keyType, accountPublicKeys.mask()))
					continue;

				switch (keyType) {
				case PublicKeyType::Linked:
					AssertPublicKeySubDocument(dbAccountPublicKeys, "linked", accountPublicKeys.linked());
					break;

				case PublicKeyType::Node:
					AssertPublicKeySubDocument(dbAccountPublicKeys, "node", accountPublicKeys.node());
					break;

				case PublicKeyType::VRF:
					AssertPublicKeySubDocument(dbAccountPublicKeys, "vrf", accountPublicKeys.vrf());
					break;

				default:
					CATAPULT_THROW_INVALID_ARGUMENT_1("unexpected key type in AccountPublicKeys", static_cast<uint16_t>(keyType));
				}

				++numExpectedFields;
			}

			if (0 != accountPublicKeys.voting().size()) {
				AssertPublicKeysSubDocument(dbAccountPublicKeys, "voting", accountPublicKeys.voting());
				++numExpectedFields;
			}

			EXPECT_EQ(numExpectedFields, GetFieldCount(dbAccountPublicKeys));
		}

		void AssertEqualAccountImportanceSnapshots(
				const state::AccountImportanceSnapshots& snapshots,
				const bsoncxx::document::view& dbImportances) {
			size_t numImportances = 0;
			for (const auto& importanceElement : dbImportances) {
				auto importanceDocument = importanceElement.get_document();
				auto importanceHeight = GetUint64(importanceDocument.view(), "height");

				auto expectedImportance = snapshots.get(model::ImportanceHeight(importanceHeight));
				EXPECT_EQ(expectedImportance, Importance(GetUint64(importanceDocument.view(), "value")));
				++numImportances;
			}

			auto expectedNumImportances = std::count_if(snapshots.begin(), snapshots.end(), [](const auto& snapshot) {
				return model::ImportanceHeight(0) != snapshot.Height;
			});
			EXPECT_EQ(static_cast<size_t>(expectedNumImportances), numImportances);
		}

		void AssertEqualAccountActivityBuckets(
				const state::AccountActivityBuckets& buckets,
				const bsoncxx::document::view& dbActivityBuckets) {
			size_t numActivityBuckets = 0;
			for (const auto& activityBucketElement : dbActivityBuckets) {
				auto activityBucketDocument = activityBucketElement.get_document();
				auto importanceHeight = GetUint64(activityBucketDocument.view(), "startHeight");

				auto expectedBucket = buckets.get(model::ImportanceHeight(importanceHeight));
				EXPECT_EQ(expectedBucket.TotalFeesPaid, Amount(GetUint64(activityBucketDocument.view(), "totalFeesPaid")));
				EXPECT_EQ(expectedBucket.BeneficiaryCount, GetUint32(activityBucketDocument.view(), "beneficiaryCount"));
				EXPECT_EQ(expectedBucket.RawScore, GetUint64(activityBucketDocument.view(), "rawScore"));
				++numActivityBuckets;
			}

			auto expectedNumActivityBuckets = std::count_if(buckets.begin(), buckets.end(), [](const auto& bucket) {
				return model::ImportanceHeight(0) != bucket.StartHeight;
			});
			EXPECT_EQ(static_cast<size_t>(expectedNumActivityBuckets), numActivityBuckets);
		}
	}

	void AssertEqualAccountState(const state::AccountState& accountState, const bsoncxx::document::view& dbAccount) {
		EXPECT_EQ(10u, GetFieldCount(dbAccount));
		EXPECT_EQ(1u, GetUint32(dbAccount, "version"));

		EXPECT_EQ(accountState.Address, GetAddressValue(dbAccount, "address"));
		EXPECT_EQ(accountState.AddressHeight, Height(GetUint64(dbAccount, "addressHeight")));
		EXPECT_EQ(accountState.PublicKey, GetKeyValue(dbAccount, "publicKey"));
		EXPECT_EQ(accountState.PublicKeyHeight, Height(GetUint64(dbAccount, "publicKeyHeight")));

		EXPECT_EQ(accountState.AccountType, static_cast<state::AccountType>(GetInt32(dbAccount, "accountType")));

		AssertEqualAccountPublicKeys(accountState.SupplementalPublicKeys, dbAccount["supplementalPublicKeys"].get_document());
		AssertEqualAccountImportanceSnapshots(accountState.ImportanceSnapshots, dbAccount["importances"].get_array().value);
		AssertEqualAccountActivityBuckets(accountState.ActivityBuckets, dbAccount["activityBuckets"].get_array().value);

		auto dbMosaics = dbAccount["mosaics"].get_array().value;
		size_t numMosaics = 0;
		for (const auto& mosaicElement : dbMosaics) {
			auto mosaicDocument = mosaicElement.get_document();
			auto id = MosaicId(GetUint64(mosaicDocument.view(), "id"));
			EXPECT_EQ(accountState.Balances.get(id), Amount(GetUint64(mosaicDocument.view(), "amount")));
			++numMosaics;
		}

		EXPECT_EQ(accountState.Balances.size(), numMosaics);
	}

	void AssertEqualMockTransactionData(const mocks::MockTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
		AssertEqualTransactionData(transaction, dbTransaction);
		EXPECT_EQ(transaction.RecipientPublicKey, GetKeyValue(dbTransaction, "recipientPublicKey"));
		EXPECT_EQ_MEMORY(transaction.DataPtr(), GetBinary(dbTransaction, "data"), transaction.Data.Size);
	}

	void AssertEqualCosignatures(const std::vector<model::Cosignature>& expectedCosignatures, const bsoncxx::array::view& dbCosignatures) {
		auto iter = dbCosignatures.cbegin();
		for (const auto& expectedCosignature : expectedCosignatures) {
			auto cosignatureView = iter->get_document().view();
			EXPECT_EQ(3u, GetFieldCount(cosignatureView));
			EXPECT_EQ(expectedCosignature.Version, GetUint64(cosignatureView, "version"));
			EXPECT_EQ(expectedCosignature.SignerPublicKey, GetKeyValue(cosignatureView, "signerPublicKey"));
			EXPECT_EQ(expectedCosignature.Signature, GetSignatureValue(cosignatureView, "signature"));
			++iter;
		}
	}

	void AssertEqualReceiptData(const model::Receipt& receipt, const bsoncxx::document::view& dbReceipt) {
		EXPECT_EQ(receipt.Version, GetInt32(dbReceipt, "version"));
		EXPECT_EQ(utils::to_underlying_type(receipt.Type), GetInt32(dbReceipt, "type"));
	}
}}
