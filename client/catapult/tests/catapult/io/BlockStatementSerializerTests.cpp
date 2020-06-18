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

#include "catapult/io/BlockStatementSerializer.h"
#include "tests/test/core/BlockStatementTestUtils.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS BlockStatementSerializerTests

	namespace {
		// region DTOs

#pragma pack(push, 1)

		struct TransactionStatementWithZeroReceipts {
			model::ReceiptSource ReceiptSource;
			uint32_t NumReceipts;
		};

		struct TransactionStatementWithTwoReceipts : public TransactionStatementWithZeroReceipts {
			model::BalanceChangeReceipt Receipt1;
			model::ArtifactExpiryReceipt<MosaicId> Receipt2;
		};

		template<typename TUnresolved>
		struct ResolutionStatementWithZeroEntries {
			TUnresolved Key;
			uint32_t NumEntries;
		};

		template<typename TUnresolved, typename TResolved>
		struct ResolutionStatementWithTwoEntries : public ResolutionStatementWithZeroEntries<TUnresolved> {
			model::ReceiptSource ReceiptSource1;
			TResolved ResolvedValue1;
			model::ReceiptSource ReceiptSource2;
			TResolved ResolvedValue2;
		};

#pragma pack(pop)

		// endregion

		// region prepare statements

		template<typename T>
		void SetMarker(T& value, uint8_t byte) {
			// set both high and low bytes so that ordering is enforced for both big and little endian types
			auto* pLowByte = reinterpret_cast<uint8_t*>(&value);
			auto* pHighByte = reinterpret_cast<uint8_t*>(&value + 1) - 1;

			*pLowByte = byte;
			*pHighByte = byte;
		}

		void PrepareStatements(model::BlockStatement& blockStatement, TransactionStatementWithZeroReceipts& statement, uint8_t order = 0) {
			statement.NumReceipts = 0;
			SetMarker(statement.ReceiptSource.PrimaryId, order);

			const auto& source = statement.ReceiptSource;
			blockStatement.TransactionStatements.emplace(source, model::TransactionStatement(source));
		}

		void PrepareStatements(model::BlockStatement& blockStatement, TransactionStatementWithTwoReceipts& statement, uint8_t order = 0) {
			statement.NumReceipts = 2;
			SetMarker(statement.ReceiptSource.PrimaryId, order);

			statement.Receipt1.Size = sizeof(model::BalanceChangeReceipt);
			SetMarker(statement.Receipt1.Type, 10);

			statement.Receipt2.Size = sizeof(model::ArtifactExpiryReceipt<MosaicId>);
			SetMarker(statement.Receipt2.Type, 20);

			const auto& source = statement.ReceiptSource;
			model::TransactionStatement transactionStatement(source);
			transactionStatement.addReceipt(statement.Receipt1);
			transactionStatement.addReceipt(statement.Receipt2);
			blockStatement.TransactionStatements.emplace(source, std::move(transactionStatement));
		}

		auto& AddStatement(model::BlockStatement& blockStatement, UnresolvedAddress address) {
			return blockStatement.AddressResolutionStatements.emplace(address, model::AddressResolutionStatement(address)).first->second;
		}

		auto& AddStatement(model::BlockStatement& blockStatement, UnresolvedMosaicId mosaicId) {
			return blockStatement.MosaicResolutionStatements.emplace(mosaicId, model::MosaicResolutionStatement(mosaicId)).first->second;
		}

		template<typename TUnresolved>
		void PrepareStatements(
				model::BlockStatement& blockStatement,
				ResolutionStatementWithZeroEntries<TUnresolved>& statement,
				uint8_t order = 0) {
			statement.NumEntries = 0;
			SetMarker(statement.Key, order);

			AddStatement(blockStatement, statement.Key);
		}

		template<typename TUnresolved, typename TResolved>
		void PrepareStatements(
				model::BlockStatement& blockStatement,
				ResolutionStatementWithTwoEntries<TUnresolved, TResolved>& statement,
				uint8_t order = 0) {
			statement.NumEntries = 2;
			SetMarker(statement.Key, order);

			// receipt sources are not aligned so cannot be passed by reference to SetMarker
			*(reinterpret_cast<uint8_t*>(&statement.ReceiptSource1.PrimaryId + 1) - 1) = 1;
			*(reinterpret_cast<uint8_t*>(&statement.ReceiptSource2.PrimaryId + 1) - 1) = 2;

			auto& resolutionStatement = AddStatement(blockStatement, statement.Key);
			resolutionStatement.addResolution(statement.ResolvedValue1, statement.ReceiptSource1);
			resolutionStatement.addResolution(statement.ResolvedValue2, statement.ReceiptSource2);
		}

		// endregion

		// region prepare tests

		template<typename TZeroEntryStatement, typename TTwoEntryStatement, typename TAction, typename TSetSizes>
		void PrepareOnlyHomogenousStatementsTest(bool shouldOrder, TAction action, TSetSizes setSizes) {
			// Arrange:
			size_t statementsSize = sizeof(TZeroEntryStatement) + 2 * sizeof(TTwoEntryStatement);
			auto buffer = test::GenerateRandomVector(3 * sizeof(uint32_t) + statementsSize);
			auto offset = setSizes(buffer, statementsSize);

			// - fix up sizes and generate expected block statement
			model::BlockStatement blockStatement;
			PrepareStatements(blockStatement, reinterpret_cast<TTwoEntryStatement&>(buffer[offset]), shouldOrder ? 1 : 0);
			offset += sizeof(TTwoEntryStatement);
			PrepareStatements(blockStatement, reinterpret_cast<TZeroEntryStatement&>(buffer[offset]), shouldOrder ? 2 : 0);
			offset += sizeof(TZeroEntryStatement);
			PrepareStatements(blockStatement, reinterpret_cast<TTwoEntryStatement&>(buffer[offset]), shouldOrder ? 3 : 0);

			// Act + Assert:
			action(blockStatement, buffer);
		}

		template<typename TAction>
		void PrepareOnlyTransactionStatementsTest(bool shouldOrder, TAction action) {
			// Arrange:
			using ZeroEntryStatement = TransactionStatementWithZeroReceipts;
			using TwoEntryStatement = TransactionStatementWithTwoReceipts;

			// Act + Assert:
			PrepareOnlyHomogenousStatementsTest<ZeroEntryStatement, TwoEntryStatement>(shouldOrder, action, [](
					auto& buffer,
					auto statementsSize) {
				reinterpret_cast<uint32_t&>(buffer[0]) = 3;
				reinterpret_cast<uint32_t&>(buffer[sizeof(uint32_t) + statementsSize]) = 0;
				reinterpret_cast<uint32_t&>(buffer[2 * sizeof(uint32_t) + statementsSize]) = 0;
				return sizeof(uint32_t);
			});
		}

		template<typename TAction>
		void PrepareOnlyAddressResolutionsTest(bool shouldOrder, TAction action) {
			// Arrange:
			using ZeroEntryStatement = ResolutionStatementWithZeroEntries<UnresolvedAddress>;
			using TwoEntryStatement = ResolutionStatementWithTwoEntries<UnresolvedAddress, Address>;

			// Act + Assert:
			PrepareOnlyHomogenousStatementsTest<ZeroEntryStatement, TwoEntryStatement>(shouldOrder, action, [](
					auto& buffer,
					auto statementsSize) {
				reinterpret_cast<uint32_t&>(buffer[0]) = 0;
				reinterpret_cast<uint32_t&>(buffer[sizeof(uint32_t)]) = 3;
				reinterpret_cast<uint32_t&>(buffer[2 * sizeof(uint32_t) + statementsSize]) = 0;
				return 2 * sizeof(uint32_t);
			});
		}

		template<typename TAction>
		void PrepareOnlyMosaicResolutionsTest(bool shouldOrder, TAction action) {
			// Arrange:
			using ZeroEntryStatement = ResolutionStatementWithZeroEntries<UnresolvedMosaicId>;
			using TwoEntryStatement = ResolutionStatementWithTwoEntries<UnresolvedMosaicId, MosaicId>;

			// Act + Assert:
			PrepareOnlyHomogenousStatementsTest<ZeroEntryStatement, TwoEntryStatement>(shouldOrder, action, [](auto& buffer, auto) {
				reinterpret_cast<uint32_t&>(buffer[0]) = 0;
				reinterpret_cast<uint32_t&>(buffer[sizeof(uint32_t)]) = 0;
				reinterpret_cast<uint32_t&>(buffer[2 * sizeof(uint32_t)]) = 3;
				return 3 * sizeof(uint32_t);
			});
		}

		template<typename TAction>
		void PrepareAllStatementsTest(bool shouldOrder, TAction action) {
			// Arrange:
			model::BlockStatement aggregateBlockStatement;
			std::vector<uint8_t> aggregateBuffer;
			PrepareOnlyTransactionStatementsTest(shouldOrder, [&](auto& blockStatement, const auto& buffer) {
				aggregateBlockStatement.TransactionStatements = std::move(blockStatement.TransactionStatements);

				aggregateBuffer.resize(buffer.size() - 2 * sizeof(uint32_t));
				std::memcpy(aggregateBuffer.data(), buffer.data(), aggregateBuffer.size());
			});
			PrepareOnlyAddressResolutionsTest(shouldOrder, [&](auto& blockStatement, const auto& buffer) {
				aggregateBlockStatement.AddressResolutionStatements = std::move(blockStatement.AddressResolutionStatements);

				auto initialSize = aggregateBuffer.size();
				auto appendSize = buffer.size() - 2 * sizeof(uint32_t);
				aggregateBuffer.resize(aggregateBuffer.size() + appendSize);
				std::memcpy(aggregateBuffer.data() + initialSize, buffer.data() + sizeof(uint32_t), appendSize);
			});
			PrepareOnlyMosaicResolutionsTest(shouldOrder, [&](auto& blockStatement, const auto& buffer) {
				aggregateBlockStatement.MosaicResolutionStatements = std::move(blockStatement.MosaicResolutionStatements);

				auto initialSize = aggregateBuffer.size();
				auto appendSize = buffer.size() - 2 * sizeof(uint32_t);
				aggregateBuffer.resize(aggregateBuffer.size() + appendSize);
				std::memcpy(aggregateBuffer.data() + initialSize, buffer.data() + 2 * sizeof(uint32_t), appendSize);
			});

			// Act + Assert:
			action(aggregateBlockStatement, aggregateBuffer);
		}

		// endregion

		// region asserts

		void AssertRead(const model::BlockStatement& expectedBlockStatement, std::vector<uint8_t>& buffer) {
			// Act:
			model::BlockStatement blockStatement;
			mocks::MockMemoryStream inputStream(buffer);
			ReadBlockStatement(inputStream, blockStatement);

			// Assert:
			test::AssertEqual(expectedBlockStatement, blockStatement);
		}

		void AssertWrite(const model::BlockStatement& blockStatement, const std::vector<uint8_t>& expectedBuffer) {
			// Act:
			std::vector<uint8_t> outputBuffer;
			mocks::MockMemoryStream outputStream(outputBuffer);
			WriteBlockStatement(blockStatement, outputStream);

			// Assert:
			ASSERT_EQ(expectedBuffer.size(), outputBuffer.size());
			EXPECT_EQ_MEMORY(expectedBuffer.data(), outputBuffer.data(), expectedBuffer.size());
		}

		// endregion
	}

	// region WriteBlockStatement

	TEST(TEST_CLASS, CanWriteBlockStatementWithoutStatements) {
		AssertWrite(model::BlockStatement(), std::vector<uint8_t>(3 * sizeof(uint32_t), 0));
	}

	TEST(TEST_CLASS, CanWriteBlockStatementWithOnlyTransactionStatements) {
		// Act + Assert: ordering is required to have output in deterministic order
		PrepareOnlyTransactionStatementsTest(true, AssertWrite);
	}

	TEST(TEST_CLASS, CanWriteBlockStatementWithOnlyAddressResolutions) {
		// Act + Assert: ordering is required to have output in deterministic order
		PrepareOnlyAddressResolutionsTest(true, AssertWrite);
	}

	TEST(TEST_CLASS, CanWriteBlockStatementWithOnlyMosaicResolutions) {
		// Act + Assert: ordering is required to have output in deterministic order
		PrepareOnlyMosaicResolutionsTest(true, AssertWrite);
	}

	TEST(TEST_CLASS, CanWriteBlockStatementWithAllStatements) {
		// Act + Assert: ordering is required to have output in deterministic order
		PrepareAllStatementsTest(true, AssertWrite);
	}

	// endregion

	// region ReadBlockStatement

	TEST(TEST_CLASS, CanReadBlockStatementWithoutStatements) {
		auto buffer = std::vector<uint8_t>(3 * sizeof(uint32_t), 0);
		AssertRead(model::BlockStatement(), buffer);
	}

	TEST(TEST_CLASS, CanReadBlockStatementWithOnlyTransactionStatements) {
		PrepareOnlyTransactionStatementsTest(false, AssertRead);
	}

	TEST(TEST_CLASS, CanReadBlockStatementWithOnlyAddressResolutions) {
		PrepareOnlyAddressResolutionsTest(false, AssertRead);
	}

	TEST(TEST_CLASS, CanReadBlockStatementWithOnlyMosaicResolutions) {
		PrepareOnlyMosaicResolutionsTest(false, AssertRead);
	}

	TEST(TEST_CLASS, CanReadBlockStatementWithAllStatements) {
		PrepareAllStatementsTest(false, AssertRead);
	}

	// endregion

	// region Roundtrip

	namespace {
		void AssertCanRoundtripBlockWithStatement(const std::vector<size_t>& numStatements) {
			// Arrange:
			auto pOriginalBlockStatement = test::GenerateRandomStatements(numStatements);

			// Act:
			model::BlockStatement result;
			test::RunRoundtripBufferTest(*pOriginalBlockStatement, result, WriteBlockStatement, ReadBlockStatement);

			// Assert:
			test::AssertEqual(*pOriginalBlockStatement, result);
		}
	}

	TEST(TEST_CLASS, CanRoundtripBlockStatementWithoutStatements) {
		AssertCanRoundtripBlockWithStatement({ 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanRoundtripBlockStatementWithOnlyTransactionStatements) {
		AssertCanRoundtripBlockWithStatement({ 5, 0, 0 });
	}

	TEST(TEST_CLASS, CanRoundtripBlockStatementWithOnlyAddressResolutions) {
		AssertCanRoundtripBlockWithStatement({ 0, 8, 0 });
	}

	TEST(TEST_CLASS, CanRoundtripBlockStatementWithOnlyMosaicResolutions) {
		AssertCanRoundtripBlockWithStatement({ 0, 0, 13 });
	}

	TEST(TEST_CLASS, CanRoundtripBlockStatementWithAllStatements) {
		AssertCanRoundtripBlockWithStatement({ 5, 8, 13 });
	}

	// endregion
}}
