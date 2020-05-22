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

#pragma once
#include "AddressTestUtils.h"
#include "catapult/model/Cosignature.h"
#include "catapult/model/RangeTypes.h"
#include "tests/TestHarness.h"
#include <memory>
#include <vector>

namespace catapult { namespace test {

	using ConstTransactions = std::vector<std::shared_ptr<const model::Transaction>>;
	using MutableTransactions = std::vector<std::shared_ptr<model::Transaction>>;

	/// Hash string of the deterministic transaction.
	constexpr auto Deterministic_Transaction_Hash_String = "D9B07A005CEC59E86310BAC4B48223330CD5746621EC4AAF5943FB4F0FFE1635";

	/// Gets the default generation hash seed used in tests.
	GenerationHashSeed GetDefaultGenerationHashSeed();

	/// Generates a transaction with random data.
	std::unique_ptr<model::Transaction> GenerateRandomTransaction();

	/// Generates a transaction for a network with specified generation hash seed (\a generationHashSeed).
	std::unique_ptr<model::Transaction> GenerateRandomTransaction(const GenerationHashSeed& generationHashSeed);

	/// Generates a transaction with random data around \a signer.
	std::unique_ptr<model::Transaction> GenerateRandomTransaction(const Key& signer);

	/// Generates \a count transactions with random data.
	MutableTransactions GenerateRandomTransactions(size_t count);

	/// Gets a const transactions container composed of all the mutable transactions in \a transactions.
	ConstTransactions MakeConst(const MutableTransactions& transactions);

	/// Generates a random transaction with size \a entitySize.
	std::unique_ptr<model::Transaction> GenerateRandomTransactionWithSize(size_t entitySize);

	/// Generates a transaction with \a deadline.
	std::unique_ptr<model::Transaction> GenerateTransactionWithDeadline(Timestamp deadline);

	/// Generates a predefined transaction, i.e. this function will always return the same transaction.
	std::unique_ptr<model::Transaction> GenerateDeterministicTransaction();

	/// Creates a transaction entity range composed of \a numTransactions transactions.
	model::TransactionRange CreateTransactionEntityRange(size_t numTransactions);

	/// Copies \a transactions into an entity range.
	model::TransactionRange CreateEntityRange(const std::vector<const model::Transaction*>& transactions);

	/// Creates a random (detached) cosignature.
	model::DetachedCosignature CreateRandomDetachedCosignature();

	/// Asserts that \a expectedCosignature and \a actualCosignature are equal with optional \a message.
	void AssertCosignature(
			const model::Cosignature& expectedCosignature,
			const model::Cosignature& actualCosignature,
			const std::string& message = "");

	/// Asserts that \a expectedCosignatures and \a actualCosignatures are equivalent with optional \a message.
	void AssertCosignatures(
			const std::vector<model::Cosignature>& expectedCosignatures,
			const std::vector<model::Cosignature>& actualCosignatures,
			const std::string& message = "");

/// Adds basic transaction property tests for \a NAME transaction with custom arguments.
#define ADD_BASIC_TRANSACTION_PROPERTY_TESTS_WITH_ARGS(NAME, ...) \
	TEST(NAME##TransactionTests, TransactionHasExpectedProperties) { \
		AssertTransactionHasExpectedProperties<NAME##Transaction>(__VA_ARGS__); \
	} \
	TEST(NAME##TransactionTests, EmbeddedTransactionHasExpectedProperties) { \
		AssertTransactionHasExpectedProperties<Embedded##NAME##Transaction>(__VA_ARGS__); \
	}

/// Adds basic transaction alignment tests for \a NAME transaction.
#define ADD_BASIC_TRANSACTION_ALIGNMENT_TESTS(NAME) \
	TEST(NAME##TransactionTests, TransactionHasProperAlignment) { \
		AssertTransactionHasProperAlignment<NAME##Transaction>(); \
	} \
	TEST(NAME##TransactionTests, EmbeddedTransactionHasProperAlignment) { \
		AssertTransactionHasProperAlignment<Embedded##NAME##Transaction>(); \
	}

/// Adds basic transaction size and property tests for \a NAME transaction with custom arguments.
#define ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(NAME, ...) \
	ADD_BASIC_TRANSACTION_PROPERTY_TESTS_WITH_ARGS(NAME, __VA_ARGS__) \
	ADD_BASIC_TRANSACTION_ALIGNMENT_TESTS(NAME) \
	\
	TEST(NAME##TransactionTests, TransactionHasExpectedSize) { \
		AssertTransactionHasExpectedSize<NAME##Transaction>(sizeof(Transaction), __VA_ARGS__); \
	} \
	TEST(NAME##TransactionTests, EmbeddedTransactionHasExpectedSize) { \
		AssertTransactionHasExpectedSize<Embedded##NAME##Transaction>(sizeof(EmbeddedTransaction), __VA_ARGS__); \
	}

/// Adds basic transaction size and property tests for \a NAME transaction.
#define ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(NAME) \
	ADD_BASIC_TRANSACTION_PROPERTY_TESTS_WITH_ARGS(NAME,) \
	ADD_BASIC_TRANSACTION_ALIGNMENT_TESTS(NAME) \
	\
	TEST(NAME##TransactionTests, TransactionHasExpectedSize) { \
		AssertTransactionHasExpectedSize<NAME##Transaction>(sizeof(Transaction)); \
	} \
	TEST(NAME##TransactionTests, EmbeddedTransactionHasExpectedSize) { \
		AssertTransactionHasExpectedSize<Embedded##NAME##Transaction>(sizeof(EmbeddedTransaction)); \
	}
}}
