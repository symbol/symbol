#include "HashLockBuilder.h"

namespace catapult { namespace builders {

	HashLockBuilder::HashLockBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_hash()
	{}

	void HashLockBuilder::setMosaic(MosaicId mosaicId, Amount amount) {
		m_mosaic = model::Mosaic{ mosaicId, amount };
	}

	void HashLockBuilder::setDuration(BlockDuration duration) {
		m_duration = duration;
	}

	void HashLockBuilder::setHash(const Hash256& hash) {
		m_hash = hash;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> HashLockBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeof(TransactionType));

		// 2. set transaction fields
		pTransaction->Mosaic = m_mosaic;
		pTransaction->Duration = m_duration;
		pTransaction->Hash = m_hash;

		return pTransaction;
	}

	std::unique_ptr<HashLockBuilder::Transaction> HashLockBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<HashLockBuilder::EmbeddedTransaction> HashLockBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}
