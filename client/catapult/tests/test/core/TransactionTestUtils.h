#pragma once
#include "AddressTestUtils.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/model/RangeTypes.h"
#include "tests/TestHarness.h"
#include <memory>
#include <vector>

namespace catapult { namespace test {
	using ConstTransactions = std::vector<std::shared_ptr<const model::Transaction>>;
	using MutableTransactions = std::vector<std::shared_ptr<model::Transaction>>;

	/// The hash string of the deterministic transaction.
	constexpr auto Deterministic_Transaction_Hash_String = "8877DC5D8D21B6E007D640E703F47BD0C5E5D6D831E4F207539747F4E8D0426A";

	/// Generates a transaction with random data.
	std::unique_ptr<model::Transaction> GenerateRandomTransaction();

	/// Generates \a count transactions with random data.
	MutableTransactions GenerateRandomTransactions(size_t count);

	/// Returns const transactions container composed of all the mutable transactions in \a transactions.
	ConstTransactions MakeConst(const MutableTransactions& transactions);

	/// Generates a random transaction with size \a entitySize.
	std::unique_ptr<model::Transaction> GenerateRandomTransaction(size_t entitySize);

	/// Generates a predefined transaction, i.e. this function will always return the same transaction.
	std::unique_ptr<model::Transaction> GenerateDeterministicTransaction();

	/// Creates a copy of \a transaction.
	std::unique_ptr<model::Transaction> CopyTransaction(const model::Transaction& transaction);

	/// Creates a copy of \a transactionInfos.
	std::vector<model::TransactionInfo> CopyTransactionInfos(const std::vector<model::TransactionInfo>& transactionInfos);

	/// Policy for creating a transaction.
	struct TransactionPolicy {
		static auto Create() {
			return GenerateRandomTransaction();
		}
	};

	/// Creates a transaction entity range composed of \a numTransactions transactions.
	model::TransactionRange CreateTransactionEntityRange(size_t numTransactions);

	/// Copies \a transactions into an entity range.
	model::TransactionRange CreateEntityRange(const std::vector<const model::Transaction*>& transactions);

/// Adds basic transaction size and property tests for \a NAME transaction.
#define ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(NAME) \
	TEST(NAME##TransactionTests, EntityHasExpectedSize) { \
		AssertEntityHasExpectedSize<NAME##Transaction>(sizeof(Transaction)); \
	} \
	TEST(NAME##TransactionTests, TransactionHasExpectedProperties) { \
		AssertTransactionHasExpectedProperties<NAME##Transaction>(); \
	} \
	TEST(NAME##TransactionTests, EmbeddedEntityHasExpectedSize) { \
		AssertEntityHasExpectedSize<Embedded##NAME##Transaction>(sizeof(EmbeddedEntity)); \
	} \
	TEST(NAME##TransactionTests, EmbeddedTransactionHasExpectedProperties) { \
		AssertTransactionHasExpectedProperties<Embedded##NAME##Transaction>(); \
	}
}}
