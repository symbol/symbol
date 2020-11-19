/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
			, m_restrictionFlags()
			, m_restrictionAdditions()
			, m_restrictionDeletions()
	{}

	void AccountAddressRestrictionBuilder::setRestrictionFlags(model::AccountRestrictionFlags restrictionFlags) {
		m_restrictionFlags = restrictionFlags;
	}

	void AccountAddressRestrictionBuilder::addRestrictionAddition(const UnresolvedAddress& restrictionAddition) {
		m_restrictionAdditions.push_back(restrictionAddition);
	}

	void AccountAddressRestrictionBuilder::addRestrictionDeletion(const UnresolvedAddress& restrictionDeletion) {
		m_restrictionDeletions.push_back(restrictionDeletion);
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
		size += m_restrictionAdditions.size() * sizeof(UnresolvedAddress);
		size += m_restrictionDeletions.size() * sizeof(UnresolvedAddress);
		return size;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> AccountAddressRestrictionBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeImpl<TransactionType>());

		// 2. set fixed transaction fields
		pTransaction->RestrictionFlags = m_restrictionFlags;
		pTransaction->RestrictionAdditionsCount = utils::checked_cast<size_t, uint8_t>(m_restrictionAdditions.size());
		pTransaction->RestrictionDeletionsCount = utils::checked_cast<size_t, uint8_t>(m_restrictionDeletions.size());
		pTransaction->AccountRestrictionTransactionBody_Reserved1 = 0;

		// 3. set transaction attachments
		std::copy(m_restrictionAdditions.cbegin(), m_restrictionAdditions.cend(), pTransaction->RestrictionAdditionsPtr());
		std::copy(m_restrictionDeletions.cbegin(), m_restrictionDeletions.cend(), pTransaction->RestrictionDeletionsPtr());

		return pTransaction;
	}
}}
