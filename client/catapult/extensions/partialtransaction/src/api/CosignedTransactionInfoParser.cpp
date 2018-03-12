#include "CosignedTransactionInfoParser.h"
#include "catapult/ionet/PacketReader.h"

namespace catapult { namespace api {

	namespace {
		using CosignedTransactionInfos = std::vector<model::CosignedTransactionInfo>;
		using TransactionPredicate = predicate<const model::Transaction&>;

#define LOG_PARSE_ERROR(DESC) CATAPULT_LOG(warning) << "unable to parse '" << DESC << "'"

		bool ReadCosignedTransactionInfo(
				ionet::PacketReader& reader,
				const TransactionPredicate& isValid,
				model::CosignedTransactionInfo& transactionInfo) {
			const auto* pTag = reader.readFixed<uint16_t>();
			if (!pTag) {
				LOG_PARSE_ERROR("tag");
				return false;
			}

			// if the high bit is set, a transaction is present, otherwise, a hash is present
			if (*pTag & 0x8000) {
				const auto* pTransaction = reader.readVariable<model::Transaction>();
				if (!pTransaction || !isValid(*pTransaction)) {
					LOG_PARSE_ERROR("transaction") << " (failed validation = " << !!pTransaction << ")";
					return false;
				}

				auto pTransactionCopy = utils::MakeSharedWithSize<model::Transaction>(pTransaction->Size);
				std::memcpy(pTransactionCopy.get(), pTransaction, pTransaction->Size);
				transactionInfo.pTransaction = pTransactionCopy;

				// clear the hash
				transactionInfo.EntityHash = Hash256();
			} else {
				const auto* pHash = reader.readFixed<Hash256>();
				if (!pHash) {
					LOG_PARSE_ERROR("hash");
					return false;
				}

				transactionInfo.EntityHash = *pHash;
			}

			// read cosignatures
			auto numCosignatures = *pTag & 0x7FFF;
			for (uint16_t i = 0; i < numCosignatures; ++i) {
				const auto* pCosignature = reader.readFixed<model::Cosignature>();
				if (!pCosignature) {
					LOG_PARSE_ERROR("cosignature") << " at " << i;
					return false;
				}

				transactionInfo.Cosignatures.push_back(*pCosignature);
			}

			return true;
		}

#undef LOG_PARSE_ERROR
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
