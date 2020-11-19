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

#pragma once
#include "TransactionBuilder.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/crypto/KeyPair.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for an aggregate transaction.
	class AggregateTransactionBuilder : public TransactionBuilder {
	public:
		using EmbeddedTransactionPointer = std::unique_ptr<model::EmbeddedTransaction>;

		/// Creates an aggregate transaction builder using \a signer for the network specified by \a networkIdentifier.
		AggregateTransactionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Adds embedded transaction (\a pTransaction).
		void addTransaction(EmbeddedTransactionPointer&& pTransaction);

	public:
		/// Gets the size of aggregate transaction.
		size_t size() const;

		/// Builds a new aggregate transaction.
		std::unique_ptr<model::AggregateTransaction> build() const;

	private:
		std::vector<EmbeddedTransactionPointer> m_transactions;
	};

	/// Helper to add cosignatures to an aggregate transaction.
	class AggregateCosignatureAppender {
	public:
		/// Creates aggregate cosignature appender around aggregate transaction (\a pAggregateTransaction)
		/// for the network with the specified generation hash seed (\a generationHashSeed).
		AggregateCosignatureAppender(
				const GenerationHashSeed& generationHashSeed,
				std::unique_ptr<model::AggregateTransaction>&& pAggregateTransaction);

	public:
		/// Cosigns an aggregate \a transaction using \a cosignatory key pair.
		void cosign(const crypto::KeyPair& cosignatory);

		/// Builds an aggregate transaction with cosigatures appended.
		std::unique_ptr<model::AggregateTransaction> build() const;

	private:
		GenerationHashSeed m_generationHashSeed;
		std::unique_ptr<model::AggregateTransaction> m_pAggregateTransaction;
		Hash256 m_transactionHash;
		std::vector<model::Cosignature> m_cosignatures;
	};
}}
