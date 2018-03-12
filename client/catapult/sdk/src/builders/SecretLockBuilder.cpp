#include "SecretLockBuilder.h"

namespace catapult { namespace builders {

	SecretLockBuilder::SecretLockBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_hashAlgorithm(model::LockHashAlgorithm::Op_Sha3)
			, m_secret()
			, m_recipient()
	{}

	void SecretLockBuilder::setMosaic(MosaicId mosaicId, Amount amount) {
		m_mosaic = model::Mosaic{ mosaicId, amount };
	}

	void SecretLockBuilder::setDuration(BlockDuration duration) {
		m_duration = duration;
	}

	void SecretLockBuilder::setHashAlgorithm(model::LockHashAlgorithm hashAlgorithm) {
		m_hashAlgorithm = hashAlgorithm;
	}

	void SecretLockBuilder::setSecret(const Hash512& secret) {
		m_secret = secret;
	}

	void SecretLockBuilder::setRecipient(const Address& recipient) {
		m_recipient = recipient;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> SecretLockBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeof(TransactionType));

		// 2. set transaction fields
		pTransaction->Mosaic = m_mosaic;
		pTransaction->Duration = m_duration;
		pTransaction->HashAlgorithm = m_hashAlgorithm;
		pTransaction->Secret = m_secret;
		pTransaction->Recipient = m_recipient;

		return pTransaction;
	}

	std::unique_ptr<SecretLockBuilder::Transaction> SecretLockBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<SecretLockBuilder::EmbeddedTransaction> SecretLockBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}
