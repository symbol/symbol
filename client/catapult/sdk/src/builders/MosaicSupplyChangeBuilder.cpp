#include "MosaicSupplyChangeBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"

namespace catapult { namespace builders {

	MosaicSupplyChangeBuilder::MosaicSupplyChangeBuilder(
			model::NetworkIdentifier networkIdentifier,
			const Key& signer,
			MosaicId mosaicId)
			: TransactionBuilder(networkIdentifier, signer)
			, m_mosaicId(mosaicId)
			, m_decrease(false)
	{}

	MosaicSupplyChangeBuilder::MosaicSupplyChangeBuilder(
			model::NetworkIdentifier networkIdentifier,
			const Key& signer,
			NamespaceId namespaceId,
			const RawString& mosaicName)
			: MosaicSupplyChangeBuilder(networkIdentifier, signer, model::GenerateMosaicId(namespaceId, mosaicName))
	{}

	void MosaicSupplyChangeBuilder::setDecrease() {
		m_decrease = true;
	}

	void MosaicSupplyChangeBuilder::setDelta(Amount delta) {
		m_delta = delta;
	}

	std::unique_ptr<model::MosaicSupplyChangeTransaction> MosaicSupplyChangeBuilder::build() {
		using TransactionType = model::MosaicSupplyChangeTransaction;

		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction(sizeof(TransactionType));

		// 2. set transaction fields
		pTransaction->MosaicId = m_mosaicId;
		pTransaction->Direction = m_decrease ? model::MosaicSupplyChangeDirection::Decrease : model::MosaicSupplyChangeDirection::Increase;
		pTransaction->Delta = m_delta;

		return pTransaction;
	}
}}
