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

#include "TransferBuilder.h"

namespace catapult { namespace builders {

	TransferBuilder::TransferBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_recipient()
			, m_message()
			, m_mosaics()
	{}

	void TransferBuilder::setRecipient(const UnresolvedAddress& recipient) {
		m_recipient = recipient;
	}

	void TransferBuilder::setMessage(const RawBuffer& message) {
		if (0 == message.Size)
			CATAPULT_THROW_INVALID_ARGUMENT("argument `message` cannot be empty");

		if (!m_message.empty())
			CATAPULT_THROW_RUNTIME_ERROR("`message` field already set");

		m_message.resize(message.Size);
		m_message.assign(message.pData, message.pData + message.Size);
	}

	void TransferBuilder::addMosaic(const model::UnresolvedMosaic& mosaic) {
		InsertSorted(m_mosaics, mosaic, [](const auto& lhs, const auto& rhs) {
			return lhs.MosaicId < rhs.MosaicId;
		});
	}

	std::unique_ptr<TransferBuilder::Transaction> TransferBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<TransferBuilder::EmbeddedTransaction> TransferBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> TransferBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType);
		size += m_message.size();
		size += m_mosaics.size() * sizeof(model::UnresolvedMosaic);
		auto pTransaction = createTransaction<TransactionType>(size);

		// 2. set fixed transaction fields
		pTransaction->Recipient = m_recipient;
		pTransaction->MessageSize = utils::checked_cast<size_t, uint16_t>(m_message.size());
		pTransaction->MosaicsCount = utils::checked_cast<size_t, uint8_t>(m_mosaics.size());

		// 3. set transaction attachments
		std::copy(m_message.cbegin(), m_message.cend(), pTransaction->MessagePtr());
		std::copy(m_mosaics.cbegin(), m_mosaics.cend(), pTransaction->MosaicsPtr());

		return pTransaction;
	}
}}
