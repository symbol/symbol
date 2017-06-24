#include "ModifyMultisigAccountBuilder.h"

namespace catapult { namespace builders {

	ModifyMultisigAccountBuilder::ModifyMultisigAccountBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_minRemovalDelta(0)
			, m_minApprovalDelta(0)
	{}

	void ModifyMultisigAccountBuilder::setMinRemovalDelta(int8_t minRemovalDelta) {
		m_minRemovalDelta = minRemovalDelta;
	}

	void ModifyMultisigAccountBuilder::setMinApprovalDelta(int8_t minApprovalDelta) {
		m_minApprovalDelta = minApprovalDelta;
	}

	void ModifyMultisigAccountBuilder::addCosignatoryModification(model::CosignatoryModificationType type, const Key& key) {
		m_modifications.push_back(model::CosignatoryModification{ type, key });
	}

	std::unique_ptr<model::ModifyMultisigAccountTransaction> ModifyMultisigAccountBuilder::build() const {
		using TransactionType = model::ModifyMultisigAccountTransaction;

		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType) + m_modifications.size() * sizeof(model::CosignatoryModification);
		auto pTransaction = createTransaction(size);

		// 2. set transaction fields
		pTransaction->MinRemovalDelta = m_minRemovalDelta;
		pTransaction->MinApprovalDelta = m_minApprovalDelta;

		// 3. set sizes upfront, so that pointers are calculated correctly
		pTransaction->ModificationsCount = utils::checked_cast<size_t, uint8_t>(m_modifications.size());

		// 4. set modifications
		if (!m_modifications.empty()) {
			auto* pModification = pTransaction->ModificationsPtr();
			for (const auto& modification : m_modifications) {
				*pModification = modification;
				++pModification;
			}
		}

		return pTransaction;
	}
}}
