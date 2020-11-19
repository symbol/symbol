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

#include "NodeKeyLinkBuilder.h"

namespace catapult { namespace builders {

	NodeKeyLinkBuilder::NodeKeyLinkBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_linkedPublicKey()
			, m_linkAction()
	{}

	void NodeKeyLinkBuilder::setLinkedPublicKey(const Key& linkedPublicKey) {
		m_linkedPublicKey = linkedPublicKey;
	}

	void NodeKeyLinkBuilder::setLinkAction(model::LinkAction linkAction) {
		m_linkAction = linkAction;
	}

	size_t NodeKeyLinkBuilder::size() const {
		return sizeImpl<Transaction>();
	}

	std::unique_ptr<NodeKeyLinkBuilder::Transaction> NodeKeyLinkBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<NodeKeyLinkBuilder::EmbeddedTransaction> NodeKeyLinkBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	size_t NodeKeyLinkBuilder::sizeImpl() const {
		// calculate transaction size
		auto size = sizeof(TransactionType);
		return size;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> NodeKeyLinkBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeImpl<TransactionType>());

		// 2. set fixed transaction fields
		pTransaction->LinkedPublicKey = m_linkedPublicKey;
		pTransaction->LinkAction = m_linkAction;

		return pTransaction;
	}
}}
