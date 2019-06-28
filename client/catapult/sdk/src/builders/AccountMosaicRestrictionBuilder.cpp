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

#include "AccountMosaicRestrictionBuilder.h"

namespace catapult { namespace builders {

	AccountMosaicRestrictionBuilder::AccountMosaicRestrictionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_restrictionType()
			, m_modifications()
	{}

	void AccountMosaicRestrictionBuilder::setRestrictionType(model::AccountRestrictionType restrictionType) {
		m_restrictionType = restrictionType;
	}

	void AccountMosaicRestrictionBuilder::addModification(const model::AccountMosaicRestrictionModification& modification) {
		m_modifications.push_back(modification);
	}

	size_t AccountMosaicRestrictionBuilder::size() const {
		return sizeImpl<Transaction>();
	}

	std::unique_ptr<AccountMosaicRestrictionBuilder::Transaction> AccountMosaicRestrictionBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<AccountMosaicRestrictionBuilder::EmbeddedTransaction> AccountMosaicRestrictionBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	size_t AccountMosaicRestrictionBuilder::sizeImpl() const {
		// calculate transaction size
		auto size = sizeof(TransactionType);
		size += m_modifications.size() * sizeof(model::AccountMosaicRestrictionModification);
		return size;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> AccountMosaicRestrictionBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeImpl<TransactionType>());

		// 2. set fixed transaction fields
		pTransaction->RestrictionType = m_restrictionType;
		pTransaction->ModificationsCount = utils::checked_cast<size_t, uint8_t>(m_modifications.size());

		// 3. set transaction attachments
		std::copy(m_modifications.cbegin(), m_modifications.cend(), pTransaction->ModificationsPtr());

		return pTransaction;
	}
}}
