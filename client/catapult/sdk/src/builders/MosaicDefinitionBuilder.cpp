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
			, m_mosaicNonce()
			, m_mosaicId()
			, m_flags()
			, m_divisibility()
			, m_properties()
	{}

	void MosaicDefinitionBuilder::setMosaicNonce(MosaicNonce mosaicNonce) {
		m_mosaicNonce = mosaicNonce;
	}

	void MosaicDefinitionBuilder::setFlags(model::MosaicFlags flags) {
		m_flags = flags;
	}

	void MosaicDefinitionBuilder::setDivisibility(uint8_t divisibility) {
		m_divisibility = divisibility;
	}

	void MosaicDefinitionBuilder::addProperty(const model::MosaicProperty& property) {
		InsertSorted(m_properties, property, [](const auto& lhs, const auto& rhs) {
			return lhs.Id < rhs.Id;
		});
	}

	std::unique_ptr<MosaicDefinitionBuilder::Transaction> MosaicDefinitionBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<MosaicDefinitionBuilder::EmbeddedTransaction> MosaicDefinitionBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> MosaicDefinitionBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType);
		size += m_properties.size() * sizeof(model::MosaicProperty);
		auto pTransaction = createTransaction<TransactionType>(size);

		// 2. set fixed transaction fields
		pTransaction->MosaicNonce = m_mosaicNonce;
		pTransaction->MosaicId = model::GenerateMosaicId(signer(), m_mosaicNonce);
		pTransaction->PropertiesHeader.Count = utils::checked_cast<size_t, uint8_t>(m_properties.size());
		pTransaction->PropertiesHeader.Flags = m_flags;
		pTransaction->PropertiesHeader.Divisibility = m_divisibility;

		// 3. set transaction attachments
		std::copy(m_properties.cbegin(), m_properties.cend(), pTransaction->PropertiesPtr());

		return pTransaction;
	}
}}
