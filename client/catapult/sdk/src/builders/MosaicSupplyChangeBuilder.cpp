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

#include "MosaicSupplyChangeBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"

namespace catapult { namespace builders {

	MosaicSupplyChangeBuilder::MosaicSupplyChangeBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer, MosaicId mosaicId)
			: TransactionBuilder(networkIdentifier, signer)
			, m_mosaicId(mosaicId)
			, m_decrease(false)
	{}

	MosaicSupplyChangeBuilder::MosaicSupplyChangeBuilder(
			model::NetworkIdentifier networkIdentifier,
			const Key& signer,
			NamespaceId namespaceId,
			const RawString& mosaicName)
			: MosaicSupplyChangeBuilder(networkIdentifier, signer, model::GenerateMosaicId(namespaceId, mosaicName))
	{}

	void MosaicSupplyChangeBuilder::setDecrease() {
		m_decrease = true;
	}

	void MosaicSupplyChangeBuilder::setDelta(Amount delta) {
		m_delta = delta;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> MosaicSupplyChangeBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeof(TransactionType));

		// 2. set transaction fields
		pTransaction->MosaicId = m_mosaicId;
		pTransaction->Direction = m_decrease ? model::MosaicSupplyChangeDirection::Decrease : model::MosaicSupplyChangeDirection::Increase;
		pTransaction->Delta = m_delta;

		return pTransaction;
	}

	std::unique_ptr<MosaicSupplyChangeBuilder::Transaction> MosaicSupplyChangeBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<MosaicSupplyChangeBuilder::EmbeddedTransaction> MosaicSupplyChangeBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}
