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

#include "MosaicGlobalRestrictionBuilder.h"

namespace catapult { namespace builders {

	MosaicGlobalRestrictionBuilder::MosaicGlobalRestrictionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_mosaicId()
			, m_referenceMosaicId()
			, m_restrictionKey()
			, m_previousRestrictionValue()
			, m_newRestrictionValue()
			, m_previousRestrictionType()
			, m_newRestrictionType()
	{}

	void MosaicGlobalRestrictionBuilder::setMosaicId(UnresolvedMosaicId mosaicId) {
		m_mosaicId = mosaicId;
	}

	void MosaicGlobalRestrictionBuilder::setReferenceMosaicId(UnresolvedMosaicId referenceMosaicId) {
		m_referenceMosaicId = referenceMosaicId;
	}

	void MosaicGlobalRestrictionBuilder::setRestrictionKey(uint64_t restrictionKey) {
		m_restrictionKey = restrictionKey;
	}

	void MosaicGlobalRestrictionBuilder::setPreviousRestrictionValue(uint64_t previousRestrictionValue) {
		m_previousRestrictionValue = previousRestrictionValue;
	}

	void MosaicGlobalRestrictionBuilder::setNewRestrictionValue(uint64_t newRestrictionValue) {
		m_newRestrictionValue = newRestrictionValue;
	}

	void MosaicGlobalRestrictionBuilder::setPreviousRestrictionType(model::MosaicRestrictionType previousRestrictionType) {
		m_previousRestrictionType = previousRestrictionType;
	}

	void MosaicGlobalRestrictionBuilder::setNewRestrictionType(model::MosaicRestrictionType newRestrictionType) {
		m_newRestrictionType = newRestrictionType;
	}

	size_t MosaicGlobalRestrictionBuilder::size() const {
		return sizeImpl<Transaction>();
	}

	std::unique_ptr<MosaicGlobalRestrictionBuilder::Transaction> MosaicGlobalRestrictionBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<MosaicGlobalRestrictionBuilder::EmbeddedTransaction> MosaicGlobalRestrictionBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	size_t MosaicGlobalRestrictionBuilder::sizeImpl() const {
		// calculate transaction size
		auto size = sizeof(TransactionType);
		return size;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> MosaicGlobalRestrictionBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeImpl<TransactionType>());

		// 2. set fixed transaction fields
		pTransaction->MosaicId = m_mosaicId;
		pTransaction->ReferenceMosaicId = m_referenceMosaicId;
		pTransaction->RestrictionKey = m_restrictionKey;
		pTransaction->PreviousRestrictionValue = m_previousRestrictionValue;
		pTransaction->NewRestrictionValue = m_newRestrictionValue;
		pTransaction->PreviousRestrictionType = m_previousRestrictionType;
		pTransaction->NewRestrictionType = m_newRestrictionType;

		return pTransaction;
	}
}}
