#include "SecretProofBuilder.h"

namespace catapult { namespace builders {

	SecretProofBuilder::SecretProofBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_hashAlgorithm(model::LockHashAlgorithm::Op_Sha3)
			, m_secret()
	{}

	void SecretProofBuilder::setHashAlgorithm(model::LockHashAlgorithm hashAlgorithm) {
		m_hashAlgorithm = hashAlgorithm;
	}

	void SecretProofBuilder::setSecret(const Hash512& secret) {
		m_secret = secret;
	}

	void SecretProofBuilder::setProof(const RawBuffer& proof) {
		m_proof.resize(proof.Size);
		std::memcpy(m_proof.data(), proof.pData, proof.Size);
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> SecretProofBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeof(TransactionType) + m_proof.size());

		// 2. set transaction fields
		pTransaction->HashAlgorithm = m_hashAlgorithm;
		pTransaction->Secret = m_secret;
		pTransaction->ProofSize = utils::checked_cast<size_t, uint16_t>(m_proof.size());

		// 3. set proof
		std::memcpy(pTransaction->ProofPtr(), m_proof.data(), m_proof.size());

		return pTransaction;
	}

	std::unique_ptr<SecretProofBuilder::Transaction> SecretProofBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<SecretProofBuilder::EmbeddedTransaction> SecretProofBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}
