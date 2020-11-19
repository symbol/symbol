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

#include "NamespaceRegistrationBuilder.h"
#include "plugins/txes/namespace/src/model/NamespaceIdGenerator.h"

namespace catapult { namespace builders {

	NamespaceRegistrationBuilder::NamespaceRegistrationBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_duration()
			, m_parentId()
			, m_id()
			, m_registrationType()
			, m_name()
	{}

	void NamespaceRegistrationBuilder::setDuration(BlockDuration duration) {
		m_duration = duration;
		m_registrationType = model::NamespaceRegistrationType::Root;
	}

	void NamespaceRegistrationBuilder::setParentId(NamespaceId parentId) {
		m_parentId = parentId;
		m_registrationType = model::NamespaceRegistrationType::Child;
	}

	void NamespaceRegistrationBuilder::setName(const RawBuffer& name) {
		if (0 == name.Size)
			CATAPULT_THROW_INVALID_ARGUMENT("argument `name` cannot be empty");

		if (!m_name.empty())
			CATAPULT_THROW_RUNTIME_ERROR("`name` field already set");

		m_name.resize(name.Size);
		m_name.assign(name.pData, name.pData + name.Size);
	}

	size_t NamespaceRegistrationBuilder::size() const {
		return sizeImpl<Transaction>();
	}

	std::unique_ptr<NamespaceRegistrationBuilder::Transaction> NamespaceRegistrationBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<NamespaceRegistrationBuilder::EmbeddedTransaction> NamespaceRegistrationBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	size_t NamespaceRegistrationBuilder::sizeImpl() const {
		// calculate transaction size
		auto size = sizeof(TransactionType);
		size += m_name.size();
		return size;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> NamespaceRegistrationBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeImpl<TransactionType>());

		// 2. set fixed transaction fields
		if (model::NamespaceRegistrationType::Root == m_registrationType)
			pTransaction->Duration = m_duration;

		if (model::NamespaceRegistrationType::Child == m_registrationType)
			pTransaction->ParentId = m_parentId;

		pTransaction->Id = model::GenerateNamespaceId(m_parentId, { reinterpret_cast<const char*>(m_name.data()), m_name.size() });
		pTransaction->RegistrationType = m_registrationType;
		pTransaction->NameSize = utils::checked_cast<size_t, uint8_t>(m_name.size());

		// 3. set transaction attachments
		std::copy(m_name.cbegin(), m_name.cend(), pTransaction->NamePtr());

		return pTransaction;
	}
}}
