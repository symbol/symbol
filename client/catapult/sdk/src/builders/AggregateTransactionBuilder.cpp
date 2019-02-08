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

#include "AggregateTransactionBuilder.h"
#include "catapult/crypto/Signer.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/utils/Functional.h"

namespace catapult { namespace builders {

	using TransactionType = model::AggregateTransaction;

	AggregateTransactionBuilder::AggregateTransactionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
	{}

	void AggregateTransactionBuilder::addTransaction(AggregateTransactionBuilder::EmbeddedTransactionPointer&& pTransaction) {
		m_pTransactions.push_back(std::move(pTransaction));
	}

	std::unique_ptr<TransactionType> AggregateTransactionBuilder::build() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto payloadSize = utils::Sum(m_pTransactions, [](const auto& pEmbeddedTransaction) { return pEmbeddedTransaction->Size; });
		auto size = sizeof(TransactionType) + payloadSize;
		auto pTransaction = createTransaction<TransactionType>(size);

		// 2. set transaction fields
		pTransaction->Type = model::Entity_Type_Aggregate_Bonded;
		pTransaction->PayloadSize = payloadSize;

		auto* pData = reinterpret_cast<uint8_t*>(pTransaction->TransactionsPtr());
		for (const auto& pEmbeddedTransaction : m_pTransactions) {
			std::memcpy(pData, pEmbeddedTransaction.get(), pEmbeddedTransaction->Size);
			pData += pEmbeddedTransaction->Size;
		}

		return pTransaction;
	}

	namespace {
		RawBuffer TransactionDataBuffer(const TransactionType& transaction) {
			return {
				reinterpret_cast<const uint8_t*>(&transaction) + model::VerifiableEntity::Header_Size,
				sizeof(TransactionType) - model::VerifiableEntity::Header_Size + transaction.PayloadSize
			};
		}
	}

	AggregateCosignatureAppender::AggregateCosignatureAppender(std::unique_ptr<TransactionType>&& pAggregateTransaction)
			: m_pAggregateTransaction(std::move(pAggregateTransaction))
	{}

	void AggregateCosignatureAppender::cosign(const crypto::KeyPair& cosigner) {
		if (m_cosignatures.empty()) {
			m_pAggregateTransaction->Type = model::Entity_Type_Aggregate_Complete;
			m_transactionHash = model::CalculateHash(*m_pAggregateTransaction, TransactionDataBuffer(*m_pAggregateTransaction));
		}

		model::Cosignature cosignature{ cosigner.publicKey(), {} };
		crypto::Sign(cosigner, m_transactionHash, cosignature.Signature);
		m_cosignatures.push_back(cosignature);
	}

	std::unique_ptr<TransactionType> AggregateCosignatureAppender::build() const {
		auto cosignaturesSize = sizeof(model::Cosignature) * m_cosignatures.size();
		auto size = m_pAggregateTransaction->Size + static_cast<uint32_t>(cosignaturesSize);
		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(size);

		std::memcpy(static_cast<void*>(pTransaction.get()), m_pAggregateTransaction.get(), m_pAggregateTransaction->Size);
		pTransaction->Size = size;
		std::memcpy(pTransaction->CosignaturesPtr(), m_cosignatures.data(), cosignaturesSize);
		return pTransaction;
	}
}}
