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

#include "AccountAddressRestrictionBuilder.h"

namespace catapult { namespace builders {

	AccountAddressRestrictionBuilder::AccountAddressRestrictionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_restrictionType()
			, m_modifications()
	{}

	void AccountAddressRestrictionBuilder::setRestrictionType(model::AccountRestrictionType restrictionType) {
		m_restrictionType = restrictionType;
	}

	void AccountAddressRestrictionBuilder::addModification(const model::AccountAddressRestrictionModification& modification) {
		m_modifications.push_back(modification);
	}

	size_t AccountAddressRestrictionBuilder::size() const {
		return sizeImpl<Transaction>();
	}

	std::unique_ptr<AccountAddressRestrictionBuilder::Transaction> AccountAddressRestrictionBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<AccountAddressRestrictionBuilder::EmbeddedTransaction> AccountAddressRestrictionBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	size_t AccountAddressRestrictionBuilder::sizeImpl() const {
		// calculate transaction size
		auto size = sizeof(TransactionType);
		size += m_modifications.size() * sizeof(model::AccountAddressRestrictionModification);
		return size;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> AccountAddressRestrictionBuilder::buildImpl() const {
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
