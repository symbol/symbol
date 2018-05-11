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

#include "BlockTestUtils.h"
#include "EntityTestUtils.h"
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"
#include "tests/TestHarness.h"
#include <memory>

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
	}

	std::unique_ptr<model::Block> GenerateEmptyRandomBlock() {
		auto signer = GenerateKeyPair();
		ConstTransactions transactions;
		return GenerateBlockWithTransactions(signer, transactions);
	}

	std::unique_ptr<model::Block> GenerateRandomBlockWithTransactions(const ConstTransactions& transactions) {
		auto signer = GenerateKeyPair();
		return GenerateBlockWithTransactions(signer, transactions);
	}

	std::unique_ptr<model::Block> GenerateRandomBlockWithTransactions(const MutableTransactions& transactions) {
		return GenerateRandomBlockWithTransactions(MakeConst(transactions));
	}

	std::unique_ptr<model::Block> GenerateBlockWithTransactions(const crypto::KeyPair& signer, const ConstTransactions& transactions) {
		model::PreviousBlockContext context;
		auto pBlock = CreateBlock(context, Network_Identifier, signer.publicKey(), transactions);
		SignBlock(signer, *pBlock);
		return pBlock;
	}

	std::unique_ptr<model::Block> GenerateBlockWithTransactions(const crypto::KeyPair& signer, const MutableTransactions& transactions) {
		return GenerateBlockWithTransactions(signer, MakeConst(transactions));
	}

	std::unique_ptr<model::Block> GenerateBlockWithTransactions(size_t numTransactions) {
		auto transactions = GenerateRandomTransactions(numTransactions);
		return GenerateRandomBlockWithTransactions(test::MakeConst(transactions));
	}

	std::unique_ptr<model::Block> GenerateBlockWithTransactionsAtHeight(size_t numTransactions, size_t height) {
		return GenerateBlockWithTransactionsAtHeight(numTransactions, Height(height));
	}

	std::unique_ptr<model::Block> GenerateBlockWithTransactionsAtHeight(size_t numTransactions, Height height) {
		auto pBlock = GenerateBlockWithTransactions(numTransactions);
		pBlock->Height = height;
		return pBlock;
	}

	std::unique_ptr<model::Block> GenerateBlockWithTransactionsAtHeight(Height height) {
		auto pBlock = GenerateBlockWithTransactionsAtHeight(5, height);
		test::FillWithRandomData(pBlock->PreviousBlockHash);
		return pBlock;
	}

	std::unique_ptr<model::Block> GenerateBlockWithTransactions(size_t numTransactions, Height height, Timestamp timestamp) {
		auto pBlock = GenerateBlockWithTransactionsAtHeight(numTransactions, height);
		pBlock->Timestamp = timestamp;
		return pBlock;
	}

	std::unique_ptr<model::Block> GenerateVerifiableBlockAtHeight(Height height) {
		model::PreviousBlockContext context;
		auto signer = GenerateKeyPair();
		auto pBlock = CreateBlock(context, Network_Identifier, signer.publicKey(), model::Transactions());
		pBlock->Height = height;
		SignBlock(signer, *pBlock);
		return pBlock;
	}

	std::unique_ptr<model::Block> GenerateNonVerifiableBlockAtHeight(Height height) {
		auto pBlock = GenerateVerifiableBlockAtHeight(height);
		pBlock->Signature = {};
		return pBlock;
	}

	std::unique_ptr<model::Block> GenerateDeterministicBlock() {
		auto keyPair = crypto::KeyPair::FromString("A41BE076B942D915EA3330B135D35C5A959A2DCC50BBB393C6407984D4A3B564");
		ConstTransactions transactions;
		transactions.push_back(GenerateDeterministicTransaction());
		auto pBlock = test::GenerateBlockWithTransactions(keyPair, transactions);
		pBlock->Difficulty = Difficulty(123'456'789'123'456);
		pBlock->Height = Height(12345);
		pBlock->Signer = keyPair.publicKey();
		pBlock->Timestamp = Timestamp(54321);
		pBlock->PreviousBlockHash = { { static_cast<uint8_t>(123) } };
		SignBlock(keyPair, *pBlock);
		return pBlock;
	}

	std::vector<uint8_t> CreateRandomBlockBuffer(size_t numBlocks) {
		constexpr auto Entity_Size = sizeof(model::Block);
		auto buffer = GenerateRandomVector(numBlocks * Entity_Size);
		for (auto i = 0u; i < numBlocks; ++i) {
			auto& block = reinterpret_cast<model::Block&>(buffer[i * Entity_Size]);
			block.Size = Entity_Size;
			block.Type = model::Entity_Type_Block;
		}

		return buffer;
	}

	model::BlockRange CreateEntityRange(const std::vector<const model::Block*>& blocks) {
		return CreateEntityRange<model::Block>(blocks);
	}

	model::BlockRange CreateBlockEntityRange(size_t numBlocks) {
		auto buffer = CreateRandomBlockBuffer(numBlocks);
		return model::BlockRange::CopyFixed(buffer.data(), numBlocks);
	}

	std::vector<model::BlockRange> PrepareRanges(size_t count) {
		std::vector<model::BlockRange> ranges;
		for (auto i = 0u; i < count; ++i)
			ranges.push_back(CreateBlockEntityRange(3));

		return ranges;
	}

	std::unique_ptr<model::Block> CopyBlock(const model::Block& block) {
		return CopyEntity(block);
	}

	model::BlockElement BlockToBlockElement(const model::Block& block, const Hash256& hash) {
		auto blockElement = BlockToBlockElement(block);
		blockElement.EntityHash = hash;
		for (auto& transactionElement : blockElement.Transactions) {
			transactionElement.OptionalExtractedAddresses = std::make_shared<model::AddressSet>();
			transactionElement.OptionalExtractedAddresses->emplace(test::GenerateRandomData<Address_Decoded_Size>());
		}

		return blockElement;
	}

	model::BlockElement BlockToBlockElement(const model::Block& block) {
		return extensions::BlockExtensions().convertBlockToBlockElement(block, {});
	}

	namespace {
		void AssertTransactionHashes(
				const std::vector<model::TransactionElement>& expectedElements,
				const std::vector<model::TransactionElement>& actualElements) {
			ASSERT_EQ(expectedElements.size(), actualElements.size());

			auto iter = actualElements.cbegin();
			auto i = 0u;
			for (const auto& expected : expectedElements) {
				auto message = "failed at transaction " + std::to_string(++i);
				EXPECT_EQ(expected.EntityHash, iter->EntityHash) << message;
				EXPECT_EQ(expected.MerkleComponentHash, iter->MerkleComponentHash) << message;
				++iter;
			}
		}
	}

	void AssertEqual(const model::BlockElement& expectedBlockElement, const model::BlockElement& blockElement) {
		EXPECT_EQ(expectedBlockElement.Block.Signature, blockElement.Block.Signature);
		EXPECT_EQ(expectedBlockElement.Block, blockElement.Block);
		EXPECT_EQ(expectedBlockElement.EntityHash, blockElement.EntityHash);
		EXPECT_EQ(expectedBlockElement.GenerationHash, blockElement.GenerationHash);
		AssertTransactionHashes(expectedBlockElement.Transactions, blockElement.Transactions);
	}

	void SignBlock(const crypto::KeyPair& signer, model::Block& block) {
		extensions::BlockExtensions().signFullBlock(signer, block);
	}
}}
