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

#include "MongoReceiptTestUtils.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/BlockStatement.h"
#include "tests/test/core/mocks/MockReceipt.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	namespace {
		template<typename TContainer, typename TAccessor>
		auto CreateMerkleTree(const TContainer& container, TAccessor accessor) {
			crypto::MerkleHashBuilder builder;
			for (const auto& element : container)
				builder.update(accessor(element));

			std::vector<Hash256> merkleTree;
			builder.final(merkleTree);
			return merkleTree;
		}

		template<typename TTraits>
		void AssertEqualResolutionStatement(
				const typename TTraits::ResolutionStatementType& statement,
				Height height,
				const bsoncxx::document::view& statementView,
				size_t expectedFieldCount,
				size_t index) {
			auto message = "at index " + std::to_string(index);
			EXPECT_EQ(expectedFieldCount, test::GetFieldCount(statementView)) << message;

			EXPECT_EQ(height, Height(test::GetUint64(statementView, "height"))) << message;
			EXPECT_EQ(statement.unresolved(), TTraits::GetUnresolved(statementView)) << message;

			auto dbResolutions = statementView["resolutionEntries"].get_array().value;
			ASSERT_EQ(statement.size(), test::GetFieldCount(dbResolutions)) << message;

			uint8_t resolutionIndex = 0;
			for (const auto& dbResolution : dbResolutions) {
				auto message2 = message + ", resolution index " + std::to_string(resolutionIndex);
				auto resolutionView = dbResolution.get_document().view();
				EXPECT_EQ(2u, test::GetFieldCount(resolutionView)) << "resolution field count " << message2;

				auto sourceView = resolutionView["source"].get_document().view();
				test::AssertEqualSource(statement.entryAt(resolutionIndex).Source, sourceView, message2);

				auto resolvedValue = statement.entryAt(resolutionIndex).ResolvedValue;
				EXPECT_EQ(resolvedValue, TTraits::GetResolved(resolutionView)) << message2;
				++resolutionIndex;
			}
		}
	}

	std::vector<Hash256> CalculateMerkleTree(const std::vector<model::TransactionElement>& transactionElements) {
		return CreateMerkleTree(transactionElements, [](const auto& transactionElement) {
			return transactionElement.MerkleComponentHash;
		});
	}

	std::vector<Hash256> CalculateMerkleTreeFromTransactionStatements(const model::BlockStatement& blockStatement) {
		return CreateMerkleTree(blockStatement.TransactionStatements, [](const auto& pair) {
			return pair.second.hash();
		});
	}

	std::shared_ptr<model::BlockStatement> GenerateRandomOptionalStatement(size_t numStatements) {
		// it's enough to generate single type of statement, rest is covered by BlockStatementTests
		auto pBlockStatement = std::make_shared<model::BlockStatement>();
		auto& statements = pBlockStatement->TransactionStatements;
		for (auto i = 0u; i < numStatements; ++i) {
			auto source = model::ReceiptSource({ static_cast<uint32_t>(test::Random()), static_cast<uint32_t>(test::Random()) });
			statements.emplace(source, model::TransactionStatement(source));
		}

		return pBlockStatement;
	}

	void AssertEqualSource(const model::ReceiptSource& source, const bsoncxx::document::view& sourceView, const std::string& message) {
		EXPECT_EQ(2u, test::GetFieldCount(sourceView)) << "source field count " << message;
		EXPECT_EQ(source.PrimaryId, test::GetUint32(sourceView, "primaryId")) << message;
		EXPECT_EQ(source.SecondaryId, test::GetUint32(sourceView, "secondaryId")) << message;
	}

	void AssertEqualReceipt(const mocks::MockReceipt& receipt, const bsoncxx::document::view& receiptView, const std::string& message) {
		EXPECT_EQ(3u, test::GetFieldCount(receiptView)) << "receipt field count " << message;
		EXPECT_EQ(receipt.Version, test::GetUint32(receiptView, "version")) << message;
		EXPECT_EQ(utils::to_underlying_type(receipt.Type), test::GetUint32(receiptView, "type")) << message;
		EXPECT_EQ(receipt.Payload, test::GetBinaryArray<11>(receiptView, "mock_payload")) << message;
	}

	void AssertEqualTransactionStatement(
			const model::TransactionStatement& statement,
			Height height,
			const bsoncxx::document::view& statementView,
			size_t expectedFieldCount,
			size_t index) {
		auto message = "at index " + std::to_string(index);
		EXPECT_EQ(expectedFieldCount, test::GetFieldCount(statementView)) << message;

		EXPECT_EQ(height, Height(test::GetUint64(statementView, "height"))) << message;

		AssertEqualSource(statement.source(), statementView["source"].get_document().view(), message);

		auto dbReceipts = statementView["receipts"].get_array().value;
		ASSERT_EQ(statement.size(), test::GetFieldCount(dbReceipts)) << message;

		uint8_t receiptIndex = 0;
		for (const auto& dbReceipt : dbReceipts) {
			auto message2 = message + ", receipt index " + std::to_string(receiptIndex);
			auto receiptView = dbReceipt.get_document().view();
			const auto& receipt = static_cast<const mocks::MockReceipt&>(statement.receiptAt(receiptIndex));
			test::AssertEqualReceipt(receipt, receiptView, message2);
			++receiptIndex;
		}
	}

	void AssertEqualAddressResolutionStatement(
			const model::AddressResolutionStatement& statement,
			Height height,
			const bsoncxx::document::view& statementView,
			size_t expectedFieldCount,
			size_t index) {
		AssertEqualResolutionStatement<AddressResolutionTraits>(statement, height, statementView, expectedFieldCount, index);
	}

	void AssertEqualMosaicResolutionStatement(
			const model::MosaicResolutionStatement& statement,
			Height height,
			const bsoncxx::document::view& statementView,
			size_t expectedFieldCount,
			size_t index) {
		AssertEqualResolutionStatement<MosaicResolutionTraits>(statement, height, statementView, expectedFieldCount, index);
	}
}}
