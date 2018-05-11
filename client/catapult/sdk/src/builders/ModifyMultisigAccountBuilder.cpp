/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

	template<typename TransactionType>
	std::unique_ptr<TransactionType> ModifyMultisigAccountBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType) + m_modifications.size() * sizeof(model::CosignatoryModification);
		auto pTransaction = createTransaction<TransactionType>(size);

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

	std::unique_ptr<ModifyMultisigAccountBuilder::Transaction> ModifyMultisigAccountBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<ModifyMultisigAccountBuilder::EmbeddedTransaction> ModifyMultisigAccountBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}
