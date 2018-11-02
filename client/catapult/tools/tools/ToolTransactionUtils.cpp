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

#include "ToolTransactionUtils.h"
#include "catapult/builders/TransferBuilder.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/extensions/ConversionExtensions.h"
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
				const UnresolvedAddress& recipient,
				uint64_t transactionId,
				std::initializer_list<model::UnresolvedMosaic> mosaics) {
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
			std::initializer_list<model::UnresolvedMosaic> mosaics) {
		auto recipient = extensions::CopyToUnresolvedAddress(model::PublicKeyToAddress(recipientPublicKey, networkId));
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
