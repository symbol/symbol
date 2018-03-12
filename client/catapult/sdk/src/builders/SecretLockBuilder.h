#pragma once
#include "TransactionBuilder.h"
#include "plugins/txes/lock/src/model/LockTypes.h"
#include "plugins/txes/lock/src/model/SecretLockTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a secret lock transaction.
	class SecretLockBuilder : public TransactionBuilder {
	public:
		using Transaction = model::SecretLockTransaction;
		using EmbeddedTransaction = model::EmbeddedSecretLockTransaction;

		/// Creates a secret lock builder for building a secret lock transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		SecretLockBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the mosaic with \a mosaicId and \a amount.
		void setMosaic(MosaicId mosaicId, Amount amount);

		/// Sets the \a duration.
		void setDuration(BlockDuration duration);

		// Sets the \a hashAlgorithm.
		void setHashAlgorithm(model::LockHashAlgorithm hashAlgorithm);

		// Sets the \a secret.
		void setSecret(const Hash512& secret);

		// Sets the \a recipient.
		void setRecipient(const Address& recipient);

	public:
		/// Builds a new secret lock transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded secret lock transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		model::Mosaic m_mosaic;
		BlockDuration m_duration;
		model::LockHashAlgorithm m_hashAlgorithm;
		Hash512 m_secret;
		Address m_recipient;
	};
}}
