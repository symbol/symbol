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

#include "NamespaceMetadataBuilder.h"

namespace catapult { namespace builders {

	NamespaceMetadataBuilder::NamespaceMetadataBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_targetAddress()
			, m_scopedMetadataKey()
			, m_targetNamespaceId()
			, m_valueSizeDelta()
			, m_value()
	{}

	void NamespaceMetadataBuilder::setTargetAddress(const UnresolvedAddress& targetAddress) {
		m_targetAddress = targetAddress;
	}

	void NamespaceMetadataBuilder::setScopedMetadataKey(uint64_t scopedMetadataKey) {
		m_scopedMetadataKey = scopedMetadataKey;
	}

	void NamespaceMetadataBuilder::setTargetNamespaceId(NamespaceId targetNamespaceId) {
		m_targetNamespaceId = targetNamespaceId;
	}

	void NamespaceMetadataBuilder::setValueSizeDelta(int16_t valueSizeDelta) {
		m_valueSizeDelta = valueSizeDelta;
	}

	void NamespaceMetadataBuilder::setValue(const RawBuffer& value) {
		if (0 == value.Size)
			CATAPULT_THROW_INVALID_ARGUMENT("argument `value` cannot be empty");

		if (!m_value.empty())
			CATAPULT_THROW_RUNTIME_ERROR("`value` field already set");

		m_value.resize(value.Size);
		m_value.assign(value.pData, value.pData + value.Size);
	}

	size_t NamespaceMetadataBuilder::size() const {
		return sizeImpl<Transaction>();
	}

	std::unique_ptr<NamespaceMetadataBuilder::Transaction> NamespaceMetadataBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<NamespaceMetadataBuilder::EmbeddedTransaction> NamespaceMetadataBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	size_t NamespaceMetadataBuilder::sizeImpl() const {
		// calculate transaction size
		auto size = sizeof(TransactionType);
		size += m_value.size();
		return size;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> NamespaceMetadataBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeImpl<TransactionType>());

		// 2. set fixed transaction fields
		pTransaction->TargetAddress = m_targetAddress;
		pTransaction->ScopedMetadataKey = m_scopedMetadataKey;
		pTransaction->TargetNamespaceId = m_targetNamespaceId;
		pTransaction->ValueSizeDelta = m_valueSizeDelta;
		pTransaction->ValueSize = utils::checked_cast<size_t, uint16_t>(m_value.size());

		// 3. set transaction attachments
		std::copy(m_value.cbegin(), m_value.cend(), pTransaction->ValuePtr());

		return pTransaction;
	}
}}
