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
#include "src/extensions/IdGenerator.h"
#include "catapult/crypto/Hashes.h"

namespace catapult { namespace builders {

	namespace {
		RawBuffer RawStringToRawBuffer(const RawString& str) {
			return RawBuffer(reinterpret_cast<const uint8_t*>(str.pData), str.Size);
		}
	}

	TransferBuilder::TransferBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer, const UnresolvedAddress& recipient)
			: TransactionBuilder(networkIdentifier, signer)
			, m_recipient(recipient)
	{}

	void TransferBuilder::setMessage(const RawBuffer& message) {
		if (0 == message.Size)
			CATAPULT_THROW_INVALID_ARGUMENT("cannot set empty message");

		if (!m_message.empty())
			CATAPULT_THROW_RUNTIME_ERROR("message was already set");

		m_message.resize(message.Size);
		m_message.assign(message.pData, message.pData + message.Size);
	}

	void TransferBuilder::setStringMessage(const RawString& message) {
		setMessage(RawStringToRawBuffer(message));
	}

	void TransferBuilder::addMosaic(UnresolvedMosaicId mosaicId, Amount amount) {
		if (m_mosaicTransfers.cend() != m_mosaicTransfers.find(mosaicId))
			CATAPULT_THROW_RUNTIME_ERROR_1("mosaic was already added", mosaicId);

		m_mosaicTransfers.emplace(mosaicId, amount);
	}

	void TransferBuilder::addMosaic(const RawString& mosaicName, Amount amount) {
		auto mosaicId = extensions::GenerateMosaicId(mosaicName);
		addMosaic(UnresolvedMosaicId(mosaicId.unwrap()), amount);
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> TransferBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType) + m_message.size();
		size += m_mosaicTransfers.size() * sizeof(model::Mosaic);
		auto pTransaction = createTransaction<TransactionType>(size);

		// 2. set transaction fields
		pTransaction->Recipient = m_recipient;

		// 3. set sizes upfront, so that pointers are calculated correctly
		pTransaction->MessageSize = utils::checked_cast<size_t, uint16_t>(m_message.size());
		pTransaction->MosaicsCount = utils::checked_cast<size_t, uint8_t>(m_mosaicTransfers.size());

		// 4. set message
		if (!m_message.empty())
			std::copy(m_message.cbegin(), m_message.cend(), pTransaction->MessagePtr());

		// 5. set mosaics
		if (!m_mosaicTransfers.empty()) {
			auto* pMosaic = pTransaction->MosaicsPtr();
			for (const auto& mosaicTransfer : m_mosaicTransfers) {
				pMosaic->MosaicId = mosaicTransfer.first;
				pMosaic->Amount = mosaicTransfer.second;
				++pMosaic;
			}
		}

		return pTransaction;
	}

	std::unique_ptr<TransferBuilder::Transaction> TransferBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<TransferBuilder::EmbeddedTransaction> TransferBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}
