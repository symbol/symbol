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

#include "CosignedTransactionInfoParser.h"
#include "catapult/ionet/PacketReader.h"
#include "catapult/utils/IntegerMath.h"

namespace catapult { namespace api {

	namespace {
		using CosignedTransactionInfos = std::vector<model::CosignedTransactionInfo>;
		using TransactionPredicate = predicate<const model::Transaction&>;

#define CATAPULT_LOG_PARSE_ERROR(DESC) CATAPULT_LOG(warning) << "unable to parse '" << DESC << "'"

		bool ReadZeros(ionet::PacketReader& reader, size_t count) {
			for (auto i = 0u; i < count; ++i) {
				const auto* pByte = reader.readFixed<uint8_t>();
				if (!pByte || 0 != *pByte)
					return false;
			}

			return true;
		}

		bool ReadCosignedTransactionInfo(
				ionet::PacketReader& reader,
				const TransactionPredicate& isValid,
				model::CosignedTransactionInfo& transactionInfo) {
			const auto* pTag = reader.readFixed<uint16_t>();
			if (!pTag || !ReadZeros(reader, 6)) {
				CATAPULT_LOG_PARSE_ERROR("tag");
				return false;
			}

			// if the high bit is set, a transaction is present, otherwise, a hash is present
			if (*pTag & 0x8000) {
				const auto* pTransaction = reader.readVariable<model::Transaction>();
				if (!pTransaction || !isValid(*pTransaction) || !ReadZeros(reader, utils::GetPaddingSize(pTransaction->Size, 8))) {
					CATAPULT_LOG_PARSE_ERROR("transaction") << " (failed validation = " << !!pTransaction << ")";
					return false;
				}

				auto pTransactionCopy = utils::MakeSharedWithSize<model::Transaction>(pTransaction->Size);
				std::memcpy(static_cast<void*>(pTransactionCopy.get()), pTransaction, pTransaction->Size);
				transactionInfo.pTransaction = pTransactionCopy;

				// clear the hash
				transactionInfo.EntityHash = Hash256();
			} else {
				const auto* pHash = reader.readFixed<Hash256>();
				if (!pHash) {
					CATAPULT_LOG_PARSE_ERROR("hash");
					return false;
				}

				transactionInfo.EntityHash = *pHash;
			}

			// read cosignatures
			auto numCosignatures = *pTag & 0x7FFF;
			for (uint16_t i = 0; i < numCosignatures; ++i) {
				const auto* pCosignature = reader.readFixed<model::Cosignature>();
				if (!pCosignature) {
					CATAPULT_LOG_PARSE_ERROR("cosignature") << " at " << i;
					return false;
				}

				transactionInfo.Cosignatures.push_back(*pCosignature);
			}

			return true;
		}

#undef CATAPULT_LOG_PARSE_ERROR
	}

	CosignedTransactionInfos ExtractCosignedTransactionInfosFromPacket(const ionet::Packet& packet, const TransactionPredicate& isValid) {
		CosignedTransactionInfos transactionInfos;
		auto reader = ionet::PacketReader(packet);
		while (!reader.empty()) {
			model::CosignedTransactionInfo transactionInfo;
			if (!ReadCosignedTransactionInfo(reader, isValid, transactionInfo))
				return CosignedTransactionInfos();

			transactionInfos.emplace_back(std::move(transactionInfo));
		}

		return transactionInfos;
	}
}}
