#pragma once
#include "catapult/utils/MemoryUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Asserts that lock \a notification properties match with corresponding properties of \a transaction.
	template<typename TBaseLockNotification, typename TTransaction>
	void AssertBaseLockNotification(const TBaseLockNotification& notification, const TTransaction& transaction) {
		EXPECT_EQ(transaction.Signer, notification.Signer);
		EXPECT_EQ(transaction.Mosaic.MosaicId, notification.Mosaic.MosaicId);
		EXPECT_EQ(transaction.Mosaic.Amount, notification.Mosaic.Amount);
		EXPECT_EQ(transaction.Duration, notification.Duration);
	}

	/// Creates a random lock transaction based on \a TTraits.
	template<typename TTraits>
	auto CreateTransaction() {
		using TransactionType = typename TTraits::TransactionType;
		auto entitySize = sizeof(TransactionType);
		auto pTransaction = utils::MakeUniqueWithSize<typename TTraits::TransactionType>(entitySize);
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });
		pTransaction->Size = static_cast<uint32_t>(entitySize);
		return pTransaction;
	}

	/// Creates a random secret proof transaction based on \a TTraits with proof of \a proofSize bytes.
	template<typename TTraits>
	auto CreateSecretProofTransaction(uint16_t proofSize) {
		using TransactionType = typename TTraits::TransactionType;
		uint32_t entitySize = sizeof(TransactionType) + proofSize;
		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });
		pTransaction->Size = entitySize;
		pTransaction->ProofSize = proofSize;
		return pTransaction;
	}
}}
