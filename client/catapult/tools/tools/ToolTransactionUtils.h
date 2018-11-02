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

#pragma once
#include "catapult/builders/TransactionBuilder.h"
#include "catapult/extensions/TransactionExtensions.h"
#include "catapult/model/Mosaic.h"
#include "catapult/state/TimestampedHash.h"
#include <vector>

namespace catapult { namespace tools {

	/// Sets \a fee and default deadline of 1h on \a builder.
	void SetDeadlineAndFee(builders::TransactionBuilder& builder, Amount fee);

	/// Creates name from a \a prefix and \a transactionId.
	std::string CreateName(const std::string& prefix, size_t transactionId);

	/// Creates transfer transaction, identified by \a transactionId with set of \a mosaics attached,
	/// from \a signer to recipient (\a recipientPublicKey) for network (\a networkId).
	std::unique_ptr<model::Transaction> CreateSignedTransferTransaction(
			model::NetworkIdentifier networkId,
			const crypto::KeyPair& signer,
			const Key& recipientPublicKey,
			uint64_t transactionId,
			std::initializer_list<model::UnresolvedMosaic> mosaics);

	/// Creates a transaction using \a builder, prepares it for sending and signs it with \a owner.
	template<typename TBuilder>
	std::unique_ptr<typename TBuilder::Transaction> CreateSignedTransaction(const crypto::KeyPair& owner, TBuilder& builder) {
		SetDeadlineAndFee(builder, Amount(0));
		auto pTransaction = builder.build();
		extensions::SignTransaction(owner, *pTransaction);
		return pTransaction;
	}

	/// A vector of transaction shared pointers.
	using Transactions = std::vector<std::shared_ptr<const model::Transaction>>;

	/// Asserts that \a resultRange is empty. If not empty, outputs a warning for each entry.
	bool VerifyEmptyAndLogFailures(const state::TimestampedHashRange& resultRange);

	/// Asserts that each element in \a transactions has a corresponding entry in \a resultRange.
	/// Outputs a warning for each transaction that does not have an entry in \a resultRange.
	bool VerifyMatchingAndLogFailures(const state::TimestampedHashRange& resultRange, const Transactions& transactions);
}}
