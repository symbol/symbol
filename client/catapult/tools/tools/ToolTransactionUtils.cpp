#include "ToolTransactionUtils.h"
#include "catapult/builders/TransferBuilder.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/extensions/TransactionExtensions.h"
#include "catapult/model/Address.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/utils/ArraySet.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/NetworkTime.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace tools {

	void SetDeadlineAndFee(builders::TransactionBuilder& builder, Amount fee) {
		builder.setDeadline(utils::NetworkTime() + utils::TimeSpan::FromHours(1));
		builder.setFee(fee);
	}

	std::string CreateName(const std::string& prefix, size_t transactionId) {
		std::stringstream nameBuilder;
		nameBuilder << prefix;
		nameBuilder << transactionId;
		return nameBuilder.str();
	}

	namespace {
		std::unique_ptr<model::TransferTransaction> CreateUnsignedTransferTransaction(
				model::NetworkIdentifier networkId,
				const Key& signerPublicKey,
				const Address& recipient,
				uint64_t transactionId,
				std::initializer_list<model::Mosaic> mosaics) {
			builders::TransferBuilder builder(networkId, signerPublicKey, recipient);
			SetDeadlineAndFee(builder, Amount(0));

			std::vector<uint8_t> message(1 + sizeof(uint64_t));
			message[0] = 0xFF;
			*reinterpret_cast<uint64_t*>(&message[1]) = transactionId;
			builder.setMessage(message);

			for (const auto& mosaic : mosaics)
				builder.addMosaic(mosaic.MosaicId, mosaic.Amount);

			return builder.build();
		}
	}

	std::unique_ptr<model::Transaction> CreateSignedTransferTransaction(
			model::NetworkIdentifier networkId,
			const crypto::KeyPair& signer,
			const Key& recipientPublicKey,
			uint64_t transactionId,
			std::initializer_list<model::Mosaic> mosaics) {
		auto recipient = model::PublicKeyToAddress(recipientPublicKey, networkId);
		auto pTransaction = CreateUnsignedTransferTransaction(networkId, signer.publicKey(), recipient, transactionId, mosaics);
		extensions::SignTransaction(signer, *pTransaction);
		return std::move(pTransaction);
	}

	bool VerifyEmptyAndLogFailures(const state::TimestampedHashRange& resultRange) {
		if (resultRange.empty())
			return true;

		CATAPULT_LOG(warning) << "the following transactions were not included in any block:";
		for (const auto& timestampedHash : resultRange)
			CATAPULT_LOG(warning) << "deadline = " << timestampedHash.Time << ", hash = " << utils::HexFormat(timestampedHash.Hash);

		return false;
	}

	bool VerifyMatchingAndLogFailures(const state::TimestampedHashRange& resultRange, const Transactions& transactions) {
		if (transactions.size() == resultRange.size())
			return true;

		// collect all hashes
		utils::HashSet hashes;
		for (const auto& pTransaction : transactions)
			hashes.insert(model::CalculateHash(*pTransaction));

		// remove the hashes that are not in the chain
		for (const auto& timestampedHash : resultRange)
			hashes.erase(timestampedHash.Hash);

		CATAPULT_LOG(warning) << "the following transactions were unexpectedly included in a block:";
		for (const auto& hash : hashes)
			CATAPULT_LOG(warning) << "hash = " << utils::HexFormat(hash);

		return false;
	}
}}
