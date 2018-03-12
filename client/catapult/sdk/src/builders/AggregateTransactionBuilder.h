#pragma once
#include "TransactionBuilder.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for an aggregate transaction.
	class AggregateTransactionBuilder : public TransactionBuilder {
	public:
		using EmbeddedTransactionPointer = std::unique_ptr<model::EmbeddedTransaction>;

		/// Creates an aggregate transaction builder using \a signer for the network specified by \a networkIdentifier.
		explicit AggregateTransactionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Adds a \a transaction.
		void addTransaction(EmbeddedTransactionPointer&& pTransaction);

	public:
		/// Builds a new aggregate transaction.
		std::unique_ptr<model::AggregateTransaction> build() const;

	private:
		std::vector<EmbeddedTransactionPointer> m_pTransactions;
	};

	/// Helper to add cosignatures to an aggregate transaction.
	class AggregateCosignatureAppender {
	public:
		/// Creates aggregate cosignature appender around aggregate transaction (\a pAggregateTransaction).
		explicit AggregateCosignatureAppender(std::unique_ptr<model::AggregateTransaction>&& pAggregateTransaction);

	public:
		/// Cosigns an aggregate \a transaction using \a cosigner key pair.
		void cosign(const crypto::KeyPair& cosigner);

		/// Builds an aggregate transaction with cosigatures appended.
		std::unique_ptr<model::AggregateTransaction> build() const;

	private:
		std::unique_ptr<model::AggregateTransaction> m_pAggregateTransaction;
		Hash256 m_transactionHash;
		std::vector<model::Cosignature> m_cosignatures;
	};
}}
