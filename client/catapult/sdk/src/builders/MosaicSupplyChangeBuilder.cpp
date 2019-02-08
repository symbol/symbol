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

namespace catapult { namespace builders {

	MosaicSupplyChangeBuilder::MosaicSupplyChangeBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_mosaicId()
			, m_direction()
			, m_delta()
	{}

	void MosaicSupplyChangeBuilder::setMosaicId(UnresolvedMosaicId mosaicId) {
		m_mosaicId = mosaicId;
	}

	void MosaicSupplyChangeBuilder::setDirection(model::MosaicSupplyChangeDirection direction) {
		m_direction = direction;
	}

	void MosaicSupplyChangeBuilder::setDelta(Amount delta) {
		m_delta = delta;
	}

	std::unique_ptr<MosaicSupplyChangeBuilder::Transaction> MosaicSupplyChangeBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<MosaicSupplyChangeBuilder::EmbeddedTransaction> MosaicSupplyChangeBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> MosaicSupplyChangeBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType);
		auto pTransaction = createTransaction<TransactionType>(size);

		// 2. set fixed transaction fields
		pTransaction->MosaicId = m_mosaicId;
		pTransaction->Direction = m_direction;
		pTransaction->Delta = m_delta;

		return pTransaction;
	}
}}
