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
#include "catapult/io/BufferInputStreamAdapter.h"
#include "catapult/io/PodIoUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/core/mocks/MockReceipt.h"

namespace catapult { namespace io {

#define TEST_CLASS BlockStatementSerializerTests

	// region WriteBlockStatement

	namespace {
		constexpr auto NumStatementsToNumEntries(size_t numStatements) {
			return 2 * numStatements;
		}

		template<typename KeyType, typename ElementType>
		size_t CalculateStatementsSize(size_t numStatements) {
			auto statementSize =
					sizeof(KeyType) + // key
					sizeof(uint32_t) + // number of receipts / resolutions
					NumStatementsToNumEntries(numStatements) * sizeof(ElementType); // receipts / resolutions

			return
					sizeof(uint32_t) // number of statements
					+ numStatements * statementSize; // statements
		}

		size_t CalculateTransactionStatementsSize(size_t numStatements) {
			return CalculateStatementsSize<model::ReceiptSource, mocks::MockReceipt>(numStatements);
		}

		size_t CalculateAddressResolutionStatementsSize(size_t numStatements) {
			return CalculateStatementsSize<UnresolvedAddress, model::AddressResolutionStatement::ResolutionEntry>(numStatements);
		}

		size_t CalculateMosaicResolutionStatementsSize(size_t numStatements) {
			return CalculateStatementsSize<UnresolvedMosaicId, model::MosaicResolutionStatement::ResolutionEntry>(numStatements);
		}

		void SkipBytes(io::InputStream& inputStream, size_t numBytes) {
			std::vector<uint8_t> data(numBytes);
			inputStream.read(data);
		}

		template<typename KeyType, typename ElementType>
		void AssertStatements(io::InputStream& inputStream, size_t expectedNumStatements) {
			auto numStatements = io::Read32(inputStream);
			ASSERT_EQ(expectedNumStatements, numStatements);

			// content of statements is not checked, read roundtrip tests below check content
			for (auto i = 0u; i < expectedNumStatements; ++i) {
				SkipBytes(inputStream, sizeof(KeyType));
				auto numReceiptsOrResolutions = io::Read32(inputStream);
				ASSERT_EQ(NumStatementsToNumEntries(expectedNumStatements), numReceiptsOrResolutions) << "statement " << i;
				for (auto j = 0u; j < NumStatementsToNumEntries(expectedNumStatements); ++j)
					SkipBytes(inputStream, sizeof(ElementType));
			}
		}

		void AssertCanWriteBlockWithStatement(const std::vector<size_t>& numStatements) {
			// Arrange:
			auto pBlockStatement = test::GenerateRandomStatements(numStatements);

			// Act:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream("", buffer);
			WriteBlockStatement(outputStream, *pBlockStatement);

			// Assert:
			auto expectedSize =
					CalculateTransactionStatementsSize(numStatements[0])
					+ CalculateAddressResolutionStatementsSize(numStatements[1])
					+ CalculateMosaicResolutionStatementsSize(numStatements[2]);

			ASSERT_EQ(expectedSize, buffer.size());

			io::BufferInputStreamAdapter<std::vector<uint8_t>> inputStream(buffer);
			AssertStatements<model::ReceiptSource, mocks::MockReceipt>(inputStream, numStatements[0]);
			AssertStatements<UnresolvedAddress, model::AddressResolutionStatement::ResolutionEntry>(inputStream, numStatements[1]);
			AssertStatements<UnresolvedMosaicId, model::MosaicResolutionStatement::ResolutionEntry>(inputStream, numStatements[2]);
		}
	}

	TEST(TEST_CLASS, WritingEmptyBlockStatementResultsInNonEmptyFile) {
		// Arrange:
		auto pBlockStatement = test::GenerateRandomStatements({ 0, 0, 0 });

		// Act:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream("", buffer);
		WriteBlockStatement(outputStream, *pBlockStatement);

		// Assert:
		std::array<uint8_t, 12u> zero{};
		ASSERT_EQ(12u, buffer.size());
		EXPECT_EQ_MEMORY(zero.data(), buffer.data(), 12);

	}

	TEST(TEST_CLASS, CanWriteBlockStatementWithOnlyTransactionStatements) {
		AssertCanWriteBlockWithStatement({ 5, 0, 0 });
	}

	TEST(TEST_CLASS, CanWriteBlockStatementWithOnlyAddressResolutions) {
		AssertCanWriteBlockWithStatement({ 0, 8, 0 });
	}

	TEST(TEST_CLASS, CanWriteBlockStatementWithOnlyMosaicResolutions) {
		AssertCanWriteBlockWithStatement({ 0, 0, 13 });
	}

	TEST(TEST_CLASS, CanWriteBlockStatementWithAllStatements) {
		AssertCanWriteBlockWithStatement({ 5, 8, 13 });
	}

	// endregion

	// region ReadBlockStatement

	namespace {
		void AssertCanReadBlockWithStatement(const std::vector<size_t>& numStatements) {
			// Arrange:
			auto pOriginalBlockStatement = test::GenerateRandomStatements(numStatements);
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream("", buffer);
			WriteBlockStatement(outputStream, *pOriginalBlockStatement);

			// Act:
			model::BlockStatement blockStatement;
			mocks::MockMemoryStream inputStream("", buffer);
			ReadBlockStatement(inputStream, blockStatement);

			// Assert:
			test::AssertEqual(*pOriginalBlockStatement, blockStatement);
		}
	}

	TEST(TEST_CLASS, CanReadBlockStatementWithoutStatements) {
		AssertCanReadBlockWithStatement({ 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanReadBlockStatementWithOnlyTransactionStatements) {
		AssertCanReadBlockWithStatement({ 5, 0, 0 });
	}

	TEST(TEST_CLASS, CanReadBlockStatementWithOnlyAddressResolutions) {
		AssertCanReadBlockWithStatement({ 0, 8, 0 });
	}

	TEST(TEST_CLASS, CanReadBlockStatementWithOnlyMosaicResolutions) {
		AssertCanReadBlockWithStatement({ 0, 0, 13 });
	}

	TEST(TEST_CLASS, CanReadBlockStatementWithAllStatements) {
		AssertCanWriteBlockWithStatement({ 5, 8, 13 });
	}

	// endregion
}}
