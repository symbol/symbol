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

#include "AggregateTransactionBuilder.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/crypto/Signer.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/utils/IntegerMath.h"

namespace catapult { namespace builders {

	using TransactionType = model::AggregateTransaction;

	namespace {
		uint32_t CalculateAggregatePayloadSize(const std::vector<AggregateTransactionBuilder::EmbeddedTransactionPointer>& transactions) {
			uint32_t size = 0;
			for (const auto& pTransaction : transactions)
				size += pTransaction->Size + utils::GetPaddingSize(pTransaction->Size, 8);

			return size;
		}
	}

	AggregateTransactionBuilder::AggregateTransactionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
	{}

	void AggregateTransactionBuilder::addTransaction(AggregateTransactionBuilder::EmbeddedTransactionPointer&& pTransaction) {
		m_transactions.push_back(std::move(pTransaction));
	}

	size_t AggregateTransactionBuilder::size() const {
		return sizeof(TransactionType) + CalculateAggregatePayloadSize(m_transactions);
	}

	std::unique_ptr<TransactionType> AggregateTransactionBuilder::build() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(size());

		// 2. set transaction fields
		auto payloadSize = CalculateAggregatePayloadSize(m_transactions);
		pTransaction->Type = model::Entity_Type_Aggregate_Bonded;
		pTransaction->PayloadSize = payloadSize;

		crypto::MerkleHashBuilder transactionsHashBuilder(m_transactions.size());

		auto* pData = reinterpret_cast<uint8_t*>(pTransaction->TransactionsPtr());
		for (const auto& pEmbeddedTransaction : m_transactions) {
			std::memcpy(pData, pEmbeddedTransaction.get(), pEmbeddedTransaction->Size);

			Hash256 transactionHash;
			crypto::Sha3_256({ pData, pEmbeddedTransaction->Size }, transactionHash);
			transactionsHashBuilder.update(transactionHash);

			pData += pEmbeddedTransaction->Size;

			auto paddingSize = utils::GetPaddingSize(pEmbeddedTransaction->Size, 8);
			std::memset(static_cast<void*>(pData), 0, paddingSize);
			pData += paddingSize;
		}

		transactionsHashBuilder.final(pTransaction->TransactionsHash);
		return pTransaction;
	}

	namespace {
		RawBuffer TransactionDataBuffer(const TransactionType& transaction) {
			return {
				reinterpret_cast<const uint8_t*>(&transaction) + TransactionType::Header_Size,
				sizeof(TransactionType) - TransactionType::Header_Size - TransactionType::Footer_Size
			};
		}
	}

	AggregateCosignatureAppender::AggregateCosignatureAppender(
			const GenerationHashSeed& generationHashSeed,
			std::unique_ptr<TransactionType>&& pAggregateTransaction)
			: m_generationHashSeed(generationHashSeed)
			, m_pAggregateTransaction(std::move(pAggregateTransaction))
	{}

	void AggregateCosignatureAppender::cosign(const crypto::KeyPair& cosignatory) {
		if (m_cosignatures.empty()) {
			m_pAggregateTransaction->Type = model::Entity_Type_Aggregate_Complete;
			m_transactionHash = model::CalculateHash(
					*m_pAggregateTransaction,
					m_generationHashSeed,
					TransactionDataBuffer(*m_pAggregateTransaction));
		}

		model::Cosignature cosignature{ cosignatory.publicKey(), {} };
		crypto::Sign(cosignatory, m_transactionHash, cosignature.Signature);
		m_cosignatures.push_back(cosignature);
	}

	std::unique_ptr<TransactionType> AggregateCosignatureAppender::build() const {
		auto cosignaturesSize = sizeof(model::Cosignature) * m_cosignatures.size();
		auto size = m_pAggregateTransaction->Size + static_cast<uint32_t>(cosignaturesSize);
		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(size);

		std::memcpy(static_cast<void*>(pTransaction.get()), m_pAggregateTransaction.get(), m_pAggregateTransaction->Size);
		pTransaction->Size = size;
		std::memcpy(static_cast<void*>(pTransaction->CosignaturesPtr()), m_cosignatures.data(), cosignaturesSize);
		return pTransaction;
	}
}}
