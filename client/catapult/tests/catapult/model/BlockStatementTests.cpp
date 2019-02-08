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

#include "catapult/model/BlockStatement.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS BlockStatementTests

	// region traits

	namespace {
		struct MerkleHashTraits {
			static constexpr auto Calculate = CalculateMerkleHash;

			static void AssertResult(const std::vector<Hash256>& expected, const Hash256& result) {
				EXPECT_EQ(expected.back(), result);
			}
		};

		struct MerkleTreeTraits {
			static constexpr auto Calculate = CalculateMerkleTree;

			static void AssertResult(const std::vector<Hash256>& expected, const std::vector<Hash256>& result) {
				EXPECT_EQ(expected.size(), result.size());
				EXPECT_EQ(expected, result);
			}
		};
	}

#define MERKLE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_MerkleHash) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MerkleHashTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MerkleTree) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MerkleTreeTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region test utils

	namespace {
		void Add(BlockStatement& statement, TransactionStatement&& transactionStatement) {
			auto source = transactionStatement.source();
			statement.TransactionStatements.emplace(source, std::move(transactionStatement));
		}

		void Add(BlockStatement& statement, AddressResolutionStatement&& resolutionStatement) {
			auto unresolved = resolutionStatement.unresolved();
			statement.AddressResolutionStatements.emplace(unresolved, std::move(resolutionStatement));
		}

		void Add(BlockStatement& statement, MosaicResolutionStatement&& resolutionStatement) {
			auto unresolved = resolutionStatement.unresolved();
			statement.MosaicResolutionStatements.emplace(unresolved, std::move(resolutionStatement));
		}
	}

	// endregion

	// region no statements

	MERKLE_TEST(CanCalculateMerkleForEmptyStatement) {
		// Arrange:
		BlockStatement statement;

		// Act:
		auto result = TTraits::Calculate(statement);

		// Assert:
		TTraits::AssertResult({ Hash256{} }, result);
	}

	// endregion

	// region single statement

	namespace {
		template<typename TTraits, typename TComponentStatement>
		void AssertCanCalculateMerkleForSingleComponentStatement(TComponentStatement&& componentStatement) {
			// Arrange:
			auto componentStatementHash = componentStatement.hash();

			BlockStatement statement;
			Add(statement, std::move(componentStatement));

			// Act:
			auto result = TTraits::Calculate(statement);

			// Assert:
			TTraits::AssertResult({ componentStatementHash }, result);
		}
	}

	MERKLE_TEST(CanCalculateMerkleForSingleTransactionStatement) {
		// Assert:
		AssertCanCalculateMerkleForSingleComponentStatement<TTraits>(TransactionStatement({ 11, 12 }));
	}

	MERKLE_TEST(CanCalculateMerkleForSingleAddressResolutionStatement) {
		// Assert:
		AssertCanCalculateMerkleForSingleComponentStatement<TTraits>(AddressResolutionStatement(UnresolvedAddress{ { { 88 } } }));
	}

	MERKLE_TEST(CanCalculateMerkleForSingleMosaicResolutionStatement) {
		// Assert:
		AssertCanCalculateMerkleForSingleComponentStatement<TTraits>(MosaicResolutionStatement(UnresolvedMosaicId(100)));
	}

	// endregion

	// region multiple statements (homogenous)

	namespace {
		std::vector<Hash256> CalculateExpectedMerkleTree(const std::vector<Hash256>& hashes) {
			crypto::MerkleHashBuilder merkleHashBuilder;
			for (const auto& hash : hashes)
				merkleHashBuilder.update(hash);

			std::vector<Hash256> merkleTree;
			merkleHashBuilder.final(merkleTree);
			return merkleTree;
		}

		template<typename TTraits, typename TComponentStatement1, typename TComponentStatement2, typename TComponentStatement3>
		void AssertCanCalculateMerkleForMultipleComponentStatements(
				TComponentStatement1&& componentStatement1,
				TComponentStatement2&& componentStatement2,
				TComponentStatement3&& componentStatement3) {
			// Arrange:
			std::vector<Hash256> componentStatementHashes{
				componentStatement1.hash(), componentStatement2.hash(), componentStatement3.hash()
			};

			BlockStatement statement;
			Add(statement, std::move(componentStatement1));
			Add(statement, std::move(componentStatement2));
			Add(statement, std::move(componentStatement3));

			// Act:
			auto result = TTraits::Calculate(statement);

			// Assert:
			TTraits::AssertResult(CalculateExpectedMerkleTree(componentStatementHashes), result);
		}
	}

	MERKLE_TEST(CanCalculateMerkleForMultipleTransactionStatements) {
		// Assert:
		AssertCanCalculateMerkleForMultipleComponentStatements<TTraits>(
				TransactionStatement({ 10, 10 }),
				TransactionStatement({ 11, 12 }),
				TransactionStatement({ 24, 11 }));
	}

	MERKLE_TEST(CanCalculateMerkleForMultipleAddressResolutionStatements) {
		// Assert:
		AssertCanCalculateMerkleForMultipleComponentStatements<TTraits>(
				AddressResolutionStatement(UnresolvedAddress{ { { 88 } } }),
				AddressResolutionStatement(UnresolvedAddress{ { { 92 } } }),
				AddressResolutionStatement(UnresolvedAddress{ { { 94 } } }));
	}

	MERKLE_TEST(CanCalculateMerkleForMultipleMosaicResolutionStatements) {
		// Assert:
		AssertCanCalculateMerkleForMultipleComponentStatements<TTraits>(
				MosaicResolutionStatement(UnresolvedMosaicId(100)),
				MosaicResolutionStatement(UnresolvedMosaicId(200)),
				MosaicResolutionStatement(UnresolvedMosaicId(300)));
	}

	// endregion

	// region multiple statements (heterogenous)

	MERKLE_TEST(CanCalculateMerkleForHeterogenousComponentStatementsOneEach) {
		// Assert:
		AssertCanCalculateMerkleForMultipleComponentStatements<TTraits>(
				TransactionStatement({ 10, 10 }),
				AddressResolutionStatement(UnresolvedAddress{ { { 92 } } }),
				MosaicResolutionStatement(UnresolvedMosaicId(300)));
	}

	MERKLE_TEST(CanCalculateMerkleForHeterogenousComponentStatementsMultipleEach) {
		// Arrange:
		auto componentStatement1 = TransactionStatement({ 10, 10 });
		auto componentStatement2 = TransactionStatement({ 24, 11 });
		auto componentStatement3 = AddressResolutionStatement(UnresolvedAddress{ { { 88 } } });
		auto componentStatement4 = AddressResolutionStatement(UnresolvedAddress{ { { 92 } } });
		auto componentStatement5 = AddressResolutionStatement(UnresolvedAddress{ { { 94 } } });
		auto componentStatement6 = MosaicResolutionStatement(UnresolvedMosaicId(100));
		auto componentStatement7 = MosaicResolutionStatement(UnresolvedMosaicId(200));

		std::vector<Hash256> componentStatementHashes{
			componentStatement1.hash(), componentStatement2.hash(), componentStatement3.hash(), componentStatement4.hash(),
			componentStatement5.hash(), componentStatement6.hash(), componentStatement7.hash()
		};

		BlockStatement statement;
		Add(statement, std::move(componentStatement1));
		Add(statement, std::move(componentStatement2));
		Add(statement, std::move(componentStatement3));
		Add(statement, std::move(componentStatement4));
		Add(statement, std::move(componentStatement5));
		Add(statement, std::move(componentStatement6));
		Add(statement, std::move(componentStatement7));

		// Act:
		auto result = TTraits::Calculate(statement);

		// Assert:
		TTraits::AssertResult(CalculateExpectedMerkleTree(componentStatementHashes), result);
	}

	// endregion

	// region count + deep copy

	namespace {
		struct CountTraits {
			static void RunStatementTest(
					size_t numTransactionStatements,
					size_t numAddressResolutions,
					size_t numMosaicResolutions,
					size_t numTotalStatements) {
				// Arrange:
				std::vector<size_t> numStatements{ numTransactionStatements, numAddressResolutions, numMosaicResolutions };
				auto pBlockStatement = test::GenerateRandomStatements(numStatements);

				// Act:
				auto count = CountTotalStatements(*pBlockStatement);

				// Assert:
				EXPECT_EQ(numTotalStatements, count);
			}
		};

		struct DeepCopyTraits {
			static void RunStatementTest(
					size_t numTransactionStatements,
					size_t numAddressResolutions,
					size_t numMosaicResolutions,
					size_t) {
				// Arrange:
				std::vector<size_t> numStatements{ numTransactionStatements, numAddressResolutions, numMosaicResolutions };
				auto pBlockStatement = test::GenerateRandomStatements(numStatements);

				// Act:
				BlockStatement blockStatementCopy;
				DeepCopyTo(blockStatementCopy, *pBlockStatement);

				// Assert:
				test::AssertEqual(*pBlockStatement, blockStatementCopy);
			}
		};
	}

#define SUB_STATEMENT_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, CanCount##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CountTraits>(); } \
	TEST(TEST_CLASS, CanDeepCopy##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeepCopyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	SUB_STATEMENT_TEST(EmptyBlockStatement) {
		TTraits::RunStatementTest(0, 0, 0, 0);
	}

	SUB_STATEMENT_TEST(BlockStatementWithOnlyTransactionStatements) {
		TTraits::RunStatementTest(3, 0, 0, 3);
	}

	SUB_STATEMENT_TEST(BlockStatementWithOnlyAddressResolutions) {
		TTraits::RunStatementTest(0, 3, 0, 3);
	}

	SUB_STATEMENT_TEST(BlockStatementWithOnlyMosaicResolutions) {
		TTraits::RunStatementTest(0, 0, 3, 3);
	}

	SUB_STATEMENT_TEST(BlockStatementWithAllStatements) {
		TTraits::RunStatementTest(3, 4, 5, 12);
	}

	// endregion
}}
