#include "AggregateTransactionTestUtils.h"
#include "catapult/crypto/Signer.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	std::unique_ptr<model::AggregateTransaction> CreateRandomAggregateTransactionWithCosignatures(uint32_t numCosignatures) {
		uint32_t size = sizeof(model::AggregateTransaction) + 124 + numCosignatures * sizeof(model::Cosignature);
		auto pTransaction = utils::MakeUniqueWithSize<model::AggregateTransaction>(size);
		FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), size });

		pTransaction->Size = size;
		pTransaction->Type = model::Entity_Type_Aggregate_Bonded;
		pTransaction->PayloadSize = 124;
		pTransaction->TransactionsPtr()->Size = 124;
		return pTransaction;
	}

	model::DetachedCosignature GenerateValidCosignature(const Hash256& aggregateHash) {
		model::Cosignature cosignature;
		auto keyPair = GenerateKeyPair();
		cosignature.Signer = keyPair.publicKey();
		crypto::Sign(keyPair, aggregateHash, cosignature.Signature);
		return { cosignature.Signer, cosignature.Signature, aggregateHash };
	}

	void FixCosignatures(const Hash256& aggregateHash, model::AggregateTransaction& aggregateTransaction) {
		// assigning DetachedCosignature to Cosignature works by slicing off ParentHash since the former is derived from the latter
		auto* pCosignature = aggregateTransaction.CosignaturesPtr();
		for (auto i = 0u; i < aggregateTransaction.CosignaturesCount(); ++i)
			*pCosignature++ = GenerateValidCosignature(aggregateHash);
	}

	AggregateTransactionWrapper CreateAggregateTransaction(uint8_t numTransactions) {
		using TransactionType = model::AggregateTransaction;
		uint32_t entitySize = sizeof(TransactionType) + numTransactions * sizeof(mocks::EmbeddedMockTransaction);
		AggregateTransactionWrapper wrapper;
		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
		pTransaction->Size = entitySize;
		pTransaction->Type = model::Entity_Type_Aggregate_Bonded;
		pTransaction->PayloadSize = numTransactions * sizeof(mocks::EmbeddedMockTransaction);
		FillWithRandomData(pTransaction->Signer);
		FillWithRandomData(pTransaction->Signature);

		auto* pSubTransaction = static_cast<mocks::EmbeddedMockTransaction*>(pTransaction->TransactionsPtr());
		for (auto i = 0u; i < numTransactions; ++i) {
			pSubTransaction->Size = sizeof(mocks::EmbeddedMockTransaction);
			pSubTransaction->Data.Size = 0;
			pSubTransaction->Type = mocks::EmbeddedMockTransaction::Entity_Type;
			FillWithRandomData(pSubTransaction->Signer);
			FillWithRandomData(pSubTransaction->Recipient);

			wrapper.SubTransactions.push_back(pSubTransaction);
			++pSubTransaction;
		}

		wrapper.pTransaction = std::move(pTransaction);
		return wrapper;
	}

	std::unique_ptr<model::Transaction> StripCosignatures(const model::AggregateTransaction& aggregateTransaction) {
		auto pTransactionWithoutCosignatures = CopyTransaction(aggregateTransaction);
		uint32_t cosignaturesSize = static_cast<uint32_t>(aggregateTransaction.CosignaturesCount()) * sizeof(model::Cosignature);
		pTransactionWithoutCosignatures->Size -= cosignaturesSize;
		return pTransactionWithoutCosignatures;
	}

	CosignaturesMap ToMap(const std::vector<model::Cosignature>& cosignatures) {
		CosignaturesMap cosignaturesMap;
		for (const auto& cosignature : cosignatures)
			cosignaturesMap.emplace(cosignature.Signer, cosignature.Signature);

		return cosignaturesMap;
	}

	void AssertStitchedTransaction(
			const model::Transaction& stitchedTransaction,
			const model::AggregateTransaction& originalTransaction,
			const std::vector<model::Cosignature>& expectedCosignatures,
			size_t numCosignaturesToIgnore) {
		// Assert: check basic properties
		auto expectedSize = sizeof(model::AggregateTransaction) + originalTransaction.PayloadSize
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
			auto message = "cosigner " + ToHexString(pCosignature->Signer);

			auto iter = expectedCosignaturesMap.find(pCosignature->Signer);
			if (expectedCosignaturesMap.cend() != iter) {
				EXPECT_EQ(iter->first, pCosignature->Signer) << message;
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
