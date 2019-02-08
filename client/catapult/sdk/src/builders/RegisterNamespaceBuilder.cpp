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

#include "RegisterNamespaceBuilder.h"
#include "plugins/txes/namespace/src/model/NamespaceIdGenerator.h"

namespace catapult { namespace builders {

	RegisterNamespaceBuilder::RegisterNamespaceBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_namespaceType()
			, m_duration()
			, m_parentId()
			, m_namespaceId()
			, m_name()
	{}

	void RegisterNamespaceBuilder::setDuration(BlockDuration duration) {
		m_duration = duration;
		m_namespaceType = model::NamespaceType::Root;
	}

	void RegisterNamespaceBuilder::setParentId(NamespaceId parentId) {
		m_parentId = parentId;
		m_namespaceType = model::NamespaceType::Child;
	}

	void RegisterNamespaceBuilder::setName(const RawBuffer& name) {
		if (0 == name.Size)
			CATAPULT_THROW_INVALID_ARGUMENT("argument `name` cannot be empty");

		if (!m_name.empty())
			CATAPULT_THROW_RUNTIME_ERROR("`name` field already set");

		m_name.resize(name.Size);
		m_name.assign(name.pData, name.pData + name.Size);
	}

	std::unique_ptr<RegisterNamespaceBuilder::Transaction> RegisterNamespaceBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<RegisterNamespaceBuilder::EmbeddedTransaction> RegisterNamespaceBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> RegisterNamespaceBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType);
		size += m_name.size();
		auto pTransaction = createTransaction<TransactionType>(size);

		// 2. set fixed transaction fields
		pTransaction->NamespaceType = m_namespaceType;
		if (model::NamespaceType::Root == m_namespaceType)
			pTransaction->Duration = m_duration;

		if (model::NamespaceType::Child == m_namespaceType)
			pTransaction->ParentId = m_parentId;

		pTransaction->NamespaceId = model::GenerateNamespaceId(
				m_parentId,
				{ reinterpret_cast<const char*>(m_name.data()), m_name.size() });
		pTransaction->NamespaceNameSize = utils::checked_cast<size_t, uint8_t>(m_name.size());

		// 3. set transaction attachments
		std::copy(m_name.cbegin(), m_name.cend(), pTransaction->NamePtr());

		return pTransaction;
	}
}}
