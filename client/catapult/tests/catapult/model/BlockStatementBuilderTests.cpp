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

#include "catapult/model/BlockStatementBuilder.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS BlockStatementBuilderTests

	namespace {
#pragma pack(push, 1)

		template<size_t N>
		struct RandomPayloadReceipt : public Receipt {
		public:
			RandomPayloadReceipt() {
				Size = sizeof(Receipt) + N;
				Version = 2;
				Type = static_cast<ReceiptType>(N);
				test::FillWithRandomData(Payload);
			}

		public:
			std::array<uint8_t, N> Payload;
		};

#pragma pack(pop)
	}

	// region source

	TEST(TEST_CLASS, SourceIsInitiallyZeroed) {
		// Arrange:
		BlockStatementBuilder builder;

		// Act:
		const auto& source = builder.source();

		// Assert:
		EXPECT_EQ(0u, source.PrimaryId);
		EXPECT_EQ(0u, source.SecondaryId);
	}

	TEST(TEST_CLASS, CanChangeSource) {
		// Arrange:
		BlockStatementBuilder builder;

		// Act:
		builder.setSource({ 12, 11 });
		const auto& source = builder.source();

		// Assert:
		EXPECT_EQ(12u, source.PrimaryId);
		EXPECT_EQ(11u, source.SecondaryId);
	}

	TEST(TEST_CLASS, CanPopZeroedSource) {
		// Arrange:
		BlockStatementBuilder builder;

		// Act:
		builder.popSource();
		const auto& source = builder.source();

		// Assert:
		EXPECT_EQ(0u, source.PrimaryId);
		EXPECT_EQ(0u, source.SecondaryId);
	}

	TEST(TEST_CLASS, CanPopSource) {
		// Arrange:
		BlockStatementBuilder builder;
		builder.setSource({ 12, 8 });

		// Act:
		builder.popSource();
		const auto& source = builder.source();

		// Assert:
		EXPECT_EQ(11u, source.PrimaryId);
		EXPECT_EQ(0u, source.SecondaryId);
	}

	// endregion

	// region empty

	TEST(TEST_CLASS, CanCreateBlockStatementWithNoChildStatements) {
		// Arrange:
		BlockStatementBuilder builder;
		builder.setSource({ 12, 11 });

		// Act:
		auto pStatement = builder.build();

		// Assert:
		EXPECT_EQ(0u, pStatement->TransactionStatements.size());
		EXPECT_EQ(0u, pStatement->AddressResolutionStatements.size());
		EXPECT_EQ(0u, pStatement->MosaicResolutionStatements.size());
	}

	// endregion

	// region transaction statements

	namespace {
		Hash256 GetTransactionStatementHash(const BlockStatement& statement, const ReceiptSource& source) {
			return statement.TransactionStatements.find(source)->second.hash();
		}

		Hash256 CalculateTransactionStatementHash(const ReceiptSource& source, const std::vector<const Receipt*>& receipts) {
			TransactionStatement transactionStatement(source);
			for (const auto* pReceipt : receipts)
				transactionStatement.addReceipt(*pReceipt);

			return transactionStatement.hash();
		}
	}

	TEST(TEST_CLASS, CanCreateBlockStatementWithSingleTransactionStatement) {
		// Arrange:
		RandomPayloadReceipt<3> receipt1;
		RandomPayloadReceipt<4> receipt2;
		RandomPayloadReceipt<2> receipt3;

		BlockStatementBuilder builder;
		builder.setSource({ 12, 11 });
		builder.addReceipt(receipt1);
		builder.addReceipt(receipt2);
		builder.addReceipt(receipt3);

		// Act:
		auto pStatement = builder.build();

		// Assert:
		auto transactionStatementHash = CalculateTransactionStatementHash({ 12, 11 }, { &receipt1, &receipt2, &receipt3 });

		ASSERT_EQ(1u, pStatement->TransactionStatements.size());
		EXPECT_EQ(transactionStatementHash, GetTransactionStatementHash(*pStatement, { 12, 11 }));

		EXPECT_EQ(0u, pStatement->AddressResolutionStatements.size());
		EXPECT_EQ(0u, pStatement->MosaicResolutionStatements.size());
	}

	TEST(TEST_CLASS, CanCreateBlockStatementWithMultipleTransactionStatements) {
		// Arrange:
		RandomPayloadReceipt<3> receipt1;
		RandomPayloadReceipt<4> receipt2;
		RandomPayloadReceipt<2> receipt3;

		BlockStatementBuilder builder;
		builder.setSource({ 12, 11 });
		builder.addReceipt(receipt1);
		builder.addReceipt(receipt2);
		builder.setSource({ 14, 0 });
		builder.addReceipt(receipt3);

		// Act:
		auto pStatement = builder.build();

		// Assert:
		auto transactionStatementHash1 = CalculateTransactionStatementHash({ 12, 11 }, { &receipt1, &receipt2 });
		auto transactionStatementHash2 = CalculateTransactionStatementHash({ 14, 0 }, { &receipt3 });

		ASSERT_EQ(2u, pStatement->TransactionStatements.size());
		EXPECT_EQ(transactionStatementHash1, GetTransactionStatementHash(*pStatement, { 12, 11 }));
		EXPECT_EQ(transactionStatementHash2, GetTransactionStatementHash(*pStatement, { 14, 0 }));

		EXPECT_EQ(0u, pStatement->AddressResolutionStatements.size());
		EXPECT_EQ(0u, pStatement->MosaicResolutionStatements.size());
	}

	// endregion

	// region resolution statements

	namespace {
		struct AddressResolutionTraits {
			using UnresolvedType = UnresolvedAddress;
			using ResolvedType = Address;
			using StatementType = AddressResolutionStatement;

			static auto GenerateUnresolved() {
				return test::GenerateRandomUnresolvedAddress();
			}

			static auto GenerateResolved() {
				return test::GenerateRandomAddress();
			}

			static const auto& GetStatements(const BlockStatement& statement) {
				return statement.AddressResolutionStatements;
			}

			static const auto& GetOtherStatements(const BlockStatement& statement) {
				return statement.MosaicResolutionStatements;
			}
		};

		struct MosaicResolutionTraits {
			using UnresolvedType = UnresolvedMosaicId;
			using ResolvedType = MosaicId;
			using StatementType = MosaicResolutionStatement;

			static auto GenerateUnresolved() {
				return test::GenerateRandomValue<UnresolvedMosaicId>();
			}

			static auto GenerateResolved() {
				return test::GenerateRandomValue<MosaicId>();
			}

			static const auto& GetStatements(const BlockStatement& statement) {
				return statement.MosaicResolutionStatements;
			}

			static const auto& GetOtherStatements(const BlockStatement& statement) {
				return statement.AddressResolutionStatements;
			}
		};

		template<typename TTraits>
		Hash256 GetResolutionStatementHash(const BlockStatement& statement, const typename TTraits::UnresolvedType& unresolved) {
			return TTraits::GetStatements(statement).find(unresolved)->second.hash();
		}

		template<typename TTraits>
		Hash256 CalculateResolutionStatementHash(
				const typename TTraits::UnresolvedType& unresolved,
				const std::vector<std::pair<ReceiptSource, typename TTraits::ResolvedType>>& resolutionPairs) {
			typename TTraits::StatementType resolutionStatement(unresolved);
			for (const auto& pair : resolutionPairs)
				resolutionStatement.addResolution(pair.second, pair.first);

			return resolutionStatement.hash();
		}

		template<typename TTraits>
		void AssertCanCreateBlockStatementWithSingleResolutionStatement() {
			// Arrange:
			auto unresolved = TTraits::GenerateUnresolved();
			auto resolved1 = TTraits::GenerateResolved();
			auto resolved2 = TTraits::GenerateResolved();

			BlockStatementBuilder builder;
			builder.setSource({ 12, 11 });
			builder.addResolution(unresolved, resolved1);
			builder.setSource({ 14, 0 });
			builder.addResolution(unresolved, resolved1); // collapsed
			builder.setSource({ 14, 2 });
			builder.addResolution(unresolved, resolved2);

			// Act:
			auto pStatement = builder.build();

			// Assert:
			auto resolutionStatementHash = CalculateResolutionStatementHash<TTraits>(unresolved, {
				{ { 12, 11 }, resolved1 },
				{ { 14, 2 }, resolved2 }
			});

			ASSERT_EQ(1u, TTraits::GetStatements(*pStatement).size());
			EXPECT_EQ(resolutionStatementHash, GetResolutionStatementHash<TTraits>(*pStatement, unresolved));

			EXPECT_EQ(0u, pStatement->TransactionStatements.size());
			EXPECT_EQ(0u, TTraits::GetOtherStatements(*pStatement).size());
		}

		template<typename TTraits>
		void AssertCanCreateBlockStatementWithMultipleResolutionStatements() {
			// Arrange:
			auto unresolved1 = TTraits::GenerateUnresolved();
			auto unresolved2 = TTraits::GenerateUnresolved();
			auto resolved1 = TTraits::GenerateResolved();
			auto resolved2 = TTraits::GenerateResolved();
			auto resolved3 = TTraits::GenerateResolved();

			BlockStatementBuilder builder;
			builder.setSource({ 12, 11 });
			builder.addResolution(unresolved1, resolved1);
			builder.setSource({ 14, 0 });
			builder.addResolution(unresolved2, resolved2);
			builder.setSource({ 14, 2 });
			builder.addResolution(unresolved1, resolved3);

			// Act:
			auto pStatement = builder.build();

			// Assert:
			auto resolutionStatementHash1 = CalculateResolutionStatementHash<TTraits>(unresolved1, {
				{ { 12, 11 }, resolved1 },
				{ { 14, 2 }, resolved3 }
			});
			auto resolutionStatementHash2 = CalculateResolutionStatementHash<TTraits>(unresolved2, { { { 14, 0 }, resolved2 } });

			ASSERT_EQ(2u, TTraits::GetStatements(*pStatement).size());
			EXPECT_EQ(resolutionStatementHash1, GetResolutionStatementHash<TTraits>(*pStatement, unresolved1));
			EXPECT_EQ(resolutionStatementHash2, GetResolutionStatementHash<TTraits>(*pStatement, unresolved2));

			EXPECT_EQ(0u, pStatement->TransactionStatements.size());
			EXPECT_EQ(0u, TTraits::GetOtherStatements(*pStatement).size());
		}
	}

	TEST(TEST_CLASS, CanCreateBlockStatementWithSingleAddressResolutionStatement) {
		AssertCanCreateBlockStatementWithSingleResolutionStatement<AddressResolutionTraits>();
	}

	TEST(TEST_CLASS, CanCreateBlockStatementWithSingleMosaicResolutionStatement) {
		AssertCanCreateBlockStatementWithSingleResolutionStatement<MosaicResolutionTraits>();
	}

	TEST(TEST_CLASS, CanCreateBlockStatementWithMultipleAddressResolutionStatements) {
		AssertCanCreateBlockStatementWithMultipleResolutionStatements<AddressResolutionTraits>();
	}

	TEST(TEST_CLASS, CanCreateBlockStatementWithMultipleMosaicResolutionStatements) {
		AssertCanCreateBlockStatementWithMultipleResolutionStatements<MosaicResolutionTraits>();
	}

	// endregion

	// region heterogenous

	TEST(TEST_CLASS, CanCreateBlockStatementWithHeterogenousComponentStatements) {
		// Arrange:
		RandomPayloadReceipt<3> receipt1;
		RandomPayloadReceipt<4> receipt2;
		RandomPayloadReceipt<2> receipt3;

		auto unresolvedAddress1 = AddressResolutionTraits::GenerateUnresolved();
		auto unresolvedAddress2 = AddressResolutionTraits::GenerateUnresolved();
		auto resolvedAddress1 = AddressResolutionTraits::GenerateResolved();
		auto resolvedAddress2 = AddressResolutionTraits::GenerateResolved();
		auto resolvedAddress3 = AddressResolutionTraits::GenerateResolved();

		auto unresolvedMosaicId1 = MosaicResolutionTraits::GenerateUnresolved();
		auto unresolvedMosaicId2 = MosaicResolutionTraits::GenerateUnresolved();
		auto resolvedMosaicId1 = MosaicResolutionTraits::GenerateResolved();
		auto resolvedMosaicId2 = MosaicResolutionTraits::GenerateResolved();
		auto resolvedMosaicId3 = MosaicResolutionTraits::GenerateResolved();

		BlockStatementBuilder builder;
		builder.setSource({ 12, 11 });
		builder.addReceipt(receipt1);
		builder.addResolution(unresolvedAddress1, resolvedAddress1);
		builder.addResolution(unresolvedAddress2, resolvedAddress2);
		builder.addResolution(unresolvedMosaicId1, resolvedMosaicId3);
		builder.setSource({ 14, 0 });
		builder.addReceipt(receipt2);
		builder.addResolution(unresolvedAddress1, resolvedAddress3);
		builder.setSource({ 14, 1 });
		builder.addReceipt(receipt3);
		builder.addResolution(unresolvedMosaicId1, resolvedMosaicId1);
		builder.addResolution(unresolvedMosaicId2, resolvedMosaicId2);

		// Act:
		auto pStatement = builder.build();

		// Assert:
		auto transactionStatementHash1 = CalculateTransactionStatementHash({ 12, 11 }, { &receipt1 });
		auto transactionStatementHash2 = CalculateTransactionStatementHash({ 14, 0 }, { &receipt2 });
		auto transactionStatementHash3 = CalculateTransactionStatementHash({ 14, 1 }, { &receipt3 });

		auto addressResolutionStatementHash1 = CalculateResolutionStatementHash<AddressResolutionTraits>(unresolvedAddress1, {
			{ { 12, 11 }, resolvedAddress1 },
			{ { 14, 0 }, resolvedAddress3 }
		});
		auto addressResolutionStatementHash2 = CalculateResolutionStatementHash<AddressResolutionTraits>(unresolvedAddress2, {
			{ { 12, 11 }, resolvedAddress2 }
		});

		auto mosaicResolutionStatementHash1 = CalculateResolutionStatementHash<MosaicResolutionTraits>(unresolvedMosaicId1, {
			{ { 12, 11 }, resolvedMosaicId3 },
			{ { 14, 1 }, resolvedMosaicId1 }
		});
		auto mosaicResolutionStatementHash2 = CalculateResolutionStatementHash<MosaicResolutionTraits>(unresolvedMosaicId2, {
			{ { 14, 1 }, resolvedMosaicId2 }
		});

		ASSERT_EQ(3u, pStatement->TransactionStatements.size());
		EXPECT_EQ(transactionStatementHash1, GetTransactionStatementHash(*pStatement, { 12, 11 }));
		EXPECT_EQ(transactionStatementHash2, GetTransactionStatementHash(*pStatement, { 14, 0 }));
		EXPECT_EQ(transactionStatementHash3, GetTransactionStatementHash(*pStatement, { 14, 1 }));

		ASSERT_EQ(2u, pStatement->AddressResolutionStatements.size());
		EXPECT_EQ(addressResolutionStatementHash1, GetResolutionStatementHash<AddressResolutionTraits>(*pStatement, unresolvedAddress1));
		EXPECT_EQ(addressResolutionStatementHash2, GetResolutionStatementHash<AddressResolutionTraits>(*pStatement, unresolvedAddress2));

		ASSERT_EQ(2u, pStatement->MosaicResolutionStatements.size());
		EXPECT_EQ(mosaicResolutionStatementHash1, GetResolutionStatementHash<MosaicResolutionTraits>(*pStatement, unresolvedMosaicId1));
		EXPECT_EQ(mosaicResolutionStatementHash2, GetResolutionStatementHash<MosaicResolutionTraits>(*pStatement, unresolvedMosaicId2));
	}

	TEST(TEST_CLASS, CanCreateBlockStatementWithHeterogenousComponentStatementsAfterPoppingSource) {
		// Arrange:
		RandomPayloadReceipt<3> receipt1;
		RandomPayloadReceipt<4> receipt2;
		RandomPayloadReceipt<2> receipt3;

		auto unresolvedAddress1 = AddressResolutionTraits::GenerateUnresolved();
		auto unresolvedAddress2 = AddressResolutionTraits::GenerateUnresolved();
		auto resolvedAddress1 = AddressResolutionTraits::GenerateResolved();
		auto resolvedAddress2 = AddressResolutionTraits::GenerateResolved();
		auto resolvedAddress3 = AddressResolutionTraits::GenerateResolved();

		auto unresolvedMosaicId1 = MosaicResolutionTraits::GenerateUnresolved();
		auto unresolvedMosaicId2 = MosaicResolutionTraits::GenerateUnresolved();
		auto resolvedMosaicId1 = MosaicResolutionTraits::GenerateResolved();
		auto resolvedMosaicId2 = MosaicResolutionTraits::GenerateResolved();
		auto resolvedMosaicId3 = MosaicResolutionTraits::GenerateResolved();

		BlockStatementBuilder builder;
		builder.setSource({ 12, 11 });
		builder.addReceipt(receipt1);
		builder.addResolution(unresolvedAddress1, resolvedAddress1);
		builder.addResolution(unresolvedAddress2, resolvedAddress2);
		builder.addResolution(unresolvedMosaicId1, resolvedMosaicId3);
		builder.setSource({ 14, 0 });
		builder.addReceipt(receipt2);
		builder.addResolution(unresolvedAddress1, resolvedAddress3);
		builder.setSource({ 14, 1 });
		builder.addReceipt(receipt3);
		builder.addResolution(unresolvedMosaicId1, resolvedMosaicId1);
		builder.addResolution(unresolvedMosaicId2, resolvedMosaicId2);
		builder.popSource(); // pop source with primary id 14

		// Act:
		auto pStatement = builder.build();

		// Assert:
		auto transactionStatementHash1 = CalculateTransactionStatementHash({ 12, 11 }, { &receipt1 });

		auto addressResolutionStatementHash1 = CalculateResolutionStatementHash<AddressResolutionTraits>(unresolvedAddress1, {
			{ { 12, 11 }, resolvedAddress1 }
		});
		auto addressResolutionStatementHash2 = CalculateResolutionStatementHash<AddressResolutionTraits>(unresolvedAddress2, {
			{ { 12, 11 }, resolvedAddress2 }
		});

		auto mosaicResolutionStatementHash1 = CalculateResolutionStatementHash<MosaicResolutionTraits>(unresolvedMosaicId1, {
			{ { 12, 11 }, resolvedMosaicId3 }
		});

		ASSERT_EQ(1u, pStatement->TransactionStatements.size());
		EXPECT_EQ(transactionStatementHash1, GetTransactionStatementHash(*pStatement, { 12, 11 }));

		ASSERT_EQ(2u, pStatement->AddressResolutionStatements.size());
		EXPECT_EQ(addressResolutionStatementHash1, GetResolutionStatementHash<AddressResolutionTraits>(*pStatement, unresolvedAddress1));
		EXPECT_EQ(addressResolutionStatementHash2, GetResolutionStatementHash<AddressResolutionTraits>(*pStatement, unresolvedAddress2));

		ASSERT_EQ(1u, pStatement->MosaicResolutionStatements.size());
		EXPECT_EQ(mosaicResolutionStatementHash1, GetResolutionStatementHash<MosaicResolutionTraits>(*pStatement, unresolvedMosaicId1));
	}

	// endregion
}}
