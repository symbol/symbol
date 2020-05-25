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

#include "BlockStatementTestUtils.h"
#include "mocks/MockMemoryStream.h"
#include "mocks/MockReceipt.h"
#include "catapult/io/BlockStatementSerializer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		template<typename T>
		void SetSourcePrimaryId(const T&, uint32_t)
		{}

		void SetSourcePrimaryId(model::ReceiptSource& source, uint32_t id) {
			source.PrimaryId = id;
		}

		void RandomFillStatement(model::TransactionStatement& statement, size_t numReceipts) {
			for (auto i = 0u; i < numReceipts; ++i) {
				mocks::MockReceipt receipt{};
				receipt.Size = sizeof(mocks::MockReceipt);
				receipt.Type = mocks::MockReceipt::Receipt_Type;
				receipt.Payload[0] = static_cast<uint8_t>(i + 1);
				statement.addReceipt(receipt);
			}
		}

		template<typename TResolutionStatement>
		void RandomFillStatement(TResolutionStatement& statement, size_t numResolutions) {
			for (auto i = 0u; i < numResolutions; ++i) {
				typename TResolutionStatement::ResolutionEntry entry;
				test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&entry), sizeof(typename TResolutionStatement::ResolutionEntry) });
				entry.Source.PrimaryId = i + 1; // needs to be in ascending order
				statement.addResolution(entry.ResolvedValue, entry.Source);
			}
		}

		template<typename TStatementKey, typename TStatementValue>
		void GenerateRandomStatements(
				std::map<TStatementKey, TStatementValue>& statements,
				size_t numStatements,
				RandomStatementsConstraints constraints) {
			for (auto i = 0u; i < numStatements; ++i) {
				TStatementKey key;
				test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&key), sizeof(TStatementKey) });
				if (RandomStatementsConstraints::Order == constraints)
					SetSourcePrimaryId(key, 2 * i + 1);

				TStatementValue statement(key);
				RandomFillStatement(statement, numStatements * 2);
				statements.emplace(key, std::move(statement));
			}
		}
	}

	std::unique_ptr<model::BlockStatement> GenerateRandomStatements(
			const std::vector<size_t>& numStatements,
			RandomStatementsConstraints constraints) {
		auto pBlockStatement = std::make_unique<model::BlockStatement>();
		GenerateRandomStatements(pBlockStatement->TransactionStatements, numStatements[0], constraints);
		GenerateRandomStatements(pBlockStatement->AddressResolutionStatements, numStatements[1], constraints);
		GenerateRandomStatements(pBlockStatement->MosaicResolutionStatements, numStatements[2], constraints);
		return pBlockStatement;
	}

	std::vector<uint8_t> SerializeBlockStatement(const model::BlockStatement& blockStatement) {
		std::vector<uint8_t> serialized;
		mocks::MockMemoryStream stream(serialized);
		io::WriteBlockStatement(blockStatement, stream);
		return serialized;
	}

	namespace {
		void AssertKeys(const UnresolvedMosaicId& lhs, const UnresolvedMosaicId& rhs, const std::string& message) {
			EXPECT_EQ(lhs, rhs) << message;
		}

		void AssertKeys(const UnresolvedAddress& lhs, const UnresolvedAddress& rhs, const std::string& message) {
			EXPECT_EQ(lhs, rhs) << message;
		}

		void AssertKeys(const model::ReceiptSource& lhs, const model::ReceiptSource& rhs, const std::string& message) {
			EXPECT_EQ(lhs.PrimaryId, rhs.PrimaryId) << message;
			EXPECT_EQ(lhs.SecondaryId, rhs.SecondaryId) << message;
		}

		void AssertStatement(const model::TransactionStatement& lhs, const model::TransactionStatement& rhs, const std::string& message) {
			ASSERT_EQ(lhs.size(), rhs.size()) << message;
			for (auto i = 0u; i < lhs.size(); ++i) {
				auto receiptMessage = message + " receipt " + std::to_string(i);
				EXPECT_EQ(lhs.receiptAt(i), rhs.receiptAt(i)) << receiptMessage;
			}
		}

		template<typename TStatement>
		void AssertStatement(const TStatement& lhs, const TStatement& rhs, const std::string& message) {
			ASSERT_EQ(lhs.size(), rhs.size()) << message;
			for (auto i = 0u; i < lhs.size(); ++i) {
				auto receiptMessage = message + " entry " + std::to_string(i);
				EXPECT_EQ(lhs.entryAt(i).ResolvedValue, rhs.entryAt(i).ResolvedValue) << receiptMessage;
				AssertKeys(lhs.entryAt(i).Source, rhs.entryAt(i).Source, receiptMessage);
			}
		}

		template<typename TMap>
		void AssertStatements(const TMap& expectedStatements, const TMap& statements) {
			ASSERT_EQ(expectedStatements.size(), statements.size());

			auto expectedIter = expectedStatements.cbegin();
			auto iter = statements.cbegin();
			for (auto i = 0u; i < expectedStatements.size(); ++i) {
				std::string message = "statement " + std::to_string(i);
				AssertKeys(expectedIter->first, iter->first, message);
				AssertStatement(expectedIter->second, iter->second, message);
				++expectedIter;
				++iter;
			}
		}
	}

	void AssertEqual(const model::BlockStatement& expectedBlockStatement, const model::BlockStatement& blockStatement) {
		AssertStatements(expectedBlockStatement.TransactionStatements, blockStatement.TransactionStatements);
		AssertStatements(expectedBlockStatement.AddressResolutionStatements, blockStatement.AddressResolutionStatements);
		AssertStatements(expectedBlockStatement.MosaicResolutionStatements, blockStatement.MosaicResolutionStatements);

		// Sanity:
		EXPECT_EQ(model::CalculateMerkleHash(expectedBlockStatement), model::CalculateMerkleHash(blockStatement));
	}
}}
