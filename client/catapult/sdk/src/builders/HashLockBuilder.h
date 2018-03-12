#pragma once
#include "TransactionBuilder.h"
#include "plugins/txes/lock/src/model/HashLockTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a hash lock transaction.
	class HashLockBuilder : public TransactionBuilder {
	public:
		using Transaction = model::HashLockTransaction;
		using EmbeddedTransaction = model::EmbeddedHashLockTransaction;

		/// Creates a hash lock builder for building a hash lock transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		HashLockBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the mosaic with \a mosaicId and \a amount.
		void setMosaic(MosaicId mosaicId, Amount amount);

		/// Sets the \a duration.
		void setDuration(BlockDuration duration);

		// Sets the \a hash.
		void setHash(const Hash256& hash);

	public:
		/// Builds a new hash lock transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded hash lock transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		model::Mosaic m_mosaic;
		Amount m_amount;
		BlockDuration m_duration;
		Hash256 m_hash;
	};
}}
