#pragma once
#include "TransactionBuilder.h"
#include "plugins/txes/lock/src/model/LockTypes.h"
#include "plugins/txes/lock/src/model/SecretProofTransaction.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a secret proof transaction.
	class SecretProofBuilder : public TransactionBuilder {
	public:
		using Transaction = model::SecretProofTransaction;
		using EmbeddedTransaction = model::EmbeddedSecretProofTransaction;

		/// Creates a secret proof builder for building a secret proof transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		SecretProofBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		// Sets the \a hashAlgorithm.
		void setHashAlgorithm(model::LockHashAlgorithm hashAlgorithm);

		// Sets the \a secret.
		void setSecret(const Hash512& secret);

		// Sets the \a proof.
		void setProof(const RawBuffer& proof);

	public:
		/// Builds a new secret proof transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded secret proof transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		model::LockHashAlgorithm m_hashAlgorithm;
		Hash512 m_secret;
		std::vector<uint8_t> m_proof;
	};
}}
