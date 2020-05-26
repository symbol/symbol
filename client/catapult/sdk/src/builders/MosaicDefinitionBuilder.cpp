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

#include "MosaicDefinitionBuilder.h"
#include "plugins/txes/mosaic/src/model/MosaicIdGenerator.h"

namespace catapult { namespace builders {

	MosaicDefinitionBuilder::MosaicDefinitionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_id()
			, m_duration()
			, m_nonce()
			, m_flags()
			, m_divisibility()
	{}

	void MosaicDefinitionBuilder::setDuration(BlockDuration duration) {
		m_duration = duration;
	}

	void MosaicDefinitionBuilder::setNonce(MosaicNonce nonce) {
		m_nonce = nonce;
	}

	void MosaicDefinitionBuilder::setFlags(model::MosaicFlags flags) {
		m_flags = flags;
	}

	void MosaicDefinitionBuilder::setDivisibility(uint8_t divisibility) {
		m_divisibility = divisibility;
	}

	size_t MosaicDefinitionBuilder::size() const {
		return sizeImpl<Transaction>();
	}

	std::unique_ptr<MosaicDefinitionBuilder::Transaction> MosaicDefinitionBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<MosaicDefinitionBuilder::EmbeddedTransaction> MosaicDefinitionBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	size_t MosaicDefinitionBuilder::sizeImpl() const {
		// calculate transaction size
		auto size = sizeof(TransactionType);
		return size;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> MosaicDefinitionBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeImpl<TransactionType>());

		// 2. set fixed transaction fields
		pTransaction->Id = model::GenerateMosaicId(model::GetSignerAddress(*pTransaction), m_nonce);
		pTransaction->Duration = m_duration;
		pTransaction->Nonce = m_nonce;
		pTransaction->Flags = m_flags;
		pTransaction->Divisibility = m_divisibility;

		return pTransaction;
	}
}}
