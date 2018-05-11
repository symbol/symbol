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
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "plugins/txes/namespace/src/model/NamespaceConstants.h"

namespace catapult { namespace builders {

	RegisterNamespaceBuilder::RegisterNamespaceBuilder(
			model::NetworkIdentifier networkIdentifier,
			const Key& signer,
			const RawString& name)
			: TransactionBuilder(networkIdentifier, signer)
			, m_name(name.pData, name.Size) {
		if (m_name.empty())
			CATAPULT_THROW_INVALID_ARGUMENT("cannot set empty name");
	}

	void RegisterNamespaceBuilder::setDuration(BlockDuration duration) {
		m_duration = duration;
	}

	void RegisterNamespaceBuilder::setParentId(NamespaceId parentId) {
		m_parentId = parentId;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> RegisterNamespaceBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType) + m_name.size();
		auto pTransaction = createTransaction<TransactionType>(size);

		// 2. set transaction fields
		if (Namespace_Base_Id == m_parentId) {
			pTransaction->NamespaceType = model::NamespaceType::Root;
			pTransaction->Duration = m_duration;
		} else {
			pTransaction->NamespaceType = model::NamespaceType::Child;
			pTransaction->ParentId = m_parentId;
		}

		pTransaction->NamespaceId = model::GenerateNamespaceId(m_parentId, m_name);

		// 3. set name
		pTransaction->NamespaceNameSize = utils::checked_cast<size_t, uint8_t>(m_name.size());
		std::copy(m_name.cbegin(), m_name.cend(), pTransaction->NamePtr());
		return pTransaction;
	}

	std::unique_ptr<RegisterNamespaceBuilder::Transaction> RegisterNamespaceBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<RegisterNamespaceBuilder::EmbeddedTransaction> RegisterNamespaceBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}
