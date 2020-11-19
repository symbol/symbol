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

#include "TransactionStatement.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace model {

	TransactionStatement::TransactionStatement(const ReceiptSource& source) : m_source(source)
	{}

	const ReceiptSource& TransactionStatement::source() const {
		return m_source;
	}

	size_t TransactionStatement::size() const {
		return m_receipts.size();
	}

	const Receipt& TransactionStatement::receiptAt(size_t index) const {
		return *m_receipts[index];
	}

	Hash256 TransactionStatement::hash() const {
		// prepend receipt header to statement
		auto version = static_cast<uint16_t>(1);
		auto type = Receipt_Type_Transaction_Group;

		crypto::Sha3_256_Builder hashBuilder;
		hashBuilder.update({ reinterpret_cast<const uint8_t*>(&version), sizeof(uint16_t) });
		hashBuilder.update({ reinterpret_cast<const uint8_t*>(&type), sizeof(ReceiptType) });
		hashBuilder.update({ reinterpret_cast<const uint8_t*>(&m_source), sizeof(ReceiptSource) });

		auto receiptHeaderSize = sizeof(Receipt::Size);
		for (const auto& pReceipt : m_receipts) {
			hashBuilder.update({
				reinterpret_cast<const uint8_t*>(pReceipt.get()) + receiptHeaderSize,
				pReceipt->Size - receiptHeaderSize
			});
		}

		Hash256 hash;
		hashBuilder.final(hash);
		return hash;
	}

	void TransactionStatement::addReceipt(const Receipt& receipt) {
		// make a copy of the receipt
		auto pReceiptCopy = utils::MakeUniqueWithSize<Receipt>(receipt.Size);
		std::memcpy(static_cast<void*>(pReceiptCopy.get()), &receipt, receipt.Size);

		// insertion sort by receipt type
		auto iter = std::find_if(m_receipts.cbegin(), m_receipts.cend(), [&receipt](const auto& pReceipt) {
			return pReceipt->Type > receipt.Type;
		});

		m_receipts.insert(iter, std::move(pReceiptCopy));
	}
}}
