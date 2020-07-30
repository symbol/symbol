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

#include "AggregateTransactionTestUtils.h"
#include "catapult/crypto/Signer.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/preprocessor.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	std::unique_ptr<model::AggregateTransaction> CreateRandomAggregateTransactionWithCosignatures(uint32_t numCosignatures) {
		uint32_t entitySize = SizeOf32<model::AggregateTransaction>() + 124 + numCosignatures * SizeOf32<model::Cosignature>();
		auto pTransaction = utils::MakeUniqueWithSize<model::AggregateTransaction>(entitySize);
		FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });

		pTransaction->Size = entitySize;
		pTransaction->Type = model::Entity_Type_Aggregate_Bonded;
		pTransaction->PayloadSize = 124;
		pTransaction->TransactionsPtr()->Size = 124;
		return pTransaction;
	}

	model::DetachedCosignature GenerateValidCosignature(const Hash256& aggregateHash) {
		model::Cosignature cosignature;
		auto keyPair = GenerateKeyPair();
		cosignature.SignerPublicKey = keyPair.publicKey();
		crypto::Sign(keyPair, aggregateHash, cosignature.Signature);
		return { cosignature.SignerPublicKey, cosignature.Signature, aggregateHash };
	}

	void FixCosignatures(const Hash256& aggregateHash, model::AggregateTransaction& aggregateTransaction) {
		// assigning DetachedCosignature to Cosignature works by slicing off ParentHash since the former is derived from the latter
		auto* pCosignature = aggregateTransaction.CosignaturesPtr();
		for (auto i = 0u; i < aggregateTransaction.CosignaturesCount(); ++i)
			*pCosignature++ = GenerateValidCosignature(aggregateHash);
	}

	AggregateTransactionWrapper CreateAggregateTransaction(uint8_t numTransactions) {
		using TransactionType = model::AggregateTransaction;

		uint32_t transactionSize = sizeof(mocks::EmbeddedMockTransaction);
		uint32_t payloadSize = numTransactions * (transactionSize + utils::GetPaddingSize(transactionSize, 8));
		uint32_t entitySize = SizeOf32<TransactionType>() + payloadSize;

		AggregateTransactionWrapper wrapper;
		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
		pTransaction->Size = entitySize;
		pTransaction->Type = model::Entity_Type_Aggregate_Bonded;
		pTransaction->PayloadSize = payloadSize;
		FillWithRandomData(pTransaction->SignerPublicKey);
		FillWithRandomData(pTransaction->Signature);

		auto* pSubTransactionBytes = reinterpret_cast<uint8_t*>(pTransaction->TransactionsPtr());
		for (auto i = 0u; i < numTransactions; ++i) {
			auto* pSubTransaction = reinterpret_cast<mocks::EmbeddedMockTransaction*>(pSubTransactionBytes);
			pSubTransaction->Size = transactionSize;
			pSubTransaction->Data.Size = 0;
			pSubTransaction->Type = mocks::EmbeddedMockTransaction::Entity_Type;
			FillWithRandomData(pSubTransaction->SignerPublicKey);
			FillWithRandomData(pSubTransaction->RecipientPublicKey);

			wrapper.SubTransactions.push_back(pSubTransaction);
			pSubTransactionBytes += transactionSize + utils::GetPaddingSize(transactionSize, 8);
		}

		wrapper.pTransaction = std::move(pTransaction);
		return wrapper;
	}

	std::unique_ptr<model::Transaction> StripCosignatures(const model::AggregateTransaction& aggregateTransaction) {
		auto pTransactionWithoutCosignatures = CopyEntity(aggregateTransaction);
		uint32_t cosignaturesSize = static_cast<uint32_t>(aggregateTransaction.CosignaturesCount()) * SizeOf32<model::Cosignature>();
		pTransactionWithoutCosignatures->Size -= cosignaturesSize;
		return PORTABLE_MOVE(pTransactionWithoutCosignatures);
	}

	CosignaturesMap ToMap(const std::vector<model::Cosignature>& cosignatures) {
		CosignaturesMap cosignaturesMap;
		for (const auto& cosignature : cosignatures)
			cosignaturesMap.emplace(cosignature.SignerPublicKey, cosignature.Signature);

		return cosignaturesMap;
	}

	void AssertStitchedTransaction(
			const model::Transaction& stitchedTransaction,
			const model::AggregateTransaction& originalTransaction,
			const std::vector<model::Cosignature>& expectedCosignatures,
			size_t numCosignaturesToIgnore) {
		// Assert: check basic properties
		auto expectedSize =
				sizeof(model::AggregateTransaction)
				+ originalTransaction.PayloadSize
				+ static_cast<uint32_t>((expectedCosignatures.size() + numCosignaturesToIgnore) * sizeof(model::Cosignature));
		ASSERT_EQ(expectedSize, stitchedTransaction.Size);
		ASSERT_EQ(model::Entity_Type_Aggregate_Bonded, stitchedTransaction.Type);

		const auto& aggregateStitchedTransaction = static_cast<const model::AggregateTransaction&>(stitchedTransaction);

		// - check tx data
		EXPECT_EQ(*StripCosignatures(originalTransaction), *StripCosignatures(aggregateStitchedTransaction));

		// - check cosignatures
		auto expectedCosignaturesMap = ToMap(expectedCosignatures);
		auto numUnknownCosignatures = 0u;
		const auto* pCosignature = aggregateStitchedTransaction.CosignaturesPtr();
		for (auto i = 0u; i < aggregateStitchedTransaction.CosignaturesCount(); ++i) {
			auto message = "cosignatory " + ToString(pCosignature->SignerPublicKey);

			auto iter = expectedCosignaturesMap.find(pCosignature->SignerPublicKey);
			if (expectedCosignaturesMap.cend() != iter) {
				EXPECT_EQ(iter->first, pCosignature->SignerPublicKey) << message;
				EXPECT_EQ(iter->second, pCosignature->Signature) << message;
				expectedCosignaturesMap.erase(iter);
			} else {
				++numUnknownCosignatures;
			}

			++pCosignature;
		}

		EXPECT_TRUE(expectedCosignaturesMap.empty());
		EXPECT_EQ(numCosignaturesToIgnore, numUnknownCosignatures);
	}
}}
