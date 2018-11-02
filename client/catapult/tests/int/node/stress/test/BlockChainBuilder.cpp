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

#include "BlockChainBuilder.h"
#include "sdk/src/extensions/TransactionExtensions.h"
#include "catapult/chain/BlockDifficultyScorer.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/io/FileBlockStorage.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/RealTransactionFactory.h"
#include "tests/test/nodeps/MijinConstants.h"

namespace catapult { namespace test {

	BlockChainBuilder::BlockChainBuilder(const Accounts& accounts, StateHashCalculator& stateHashCalculator)
			: BlockChainBuilder(accounts, stateHashCalculator, CreateLocalNodeBlockChainConfiguration())
	{}

	BlockChainBuilder::BlockChainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockChainConfiguration& config)
			: BlockChainBuilder(accounts, stateHashCalculator, config, false)
	{}

	BlockChainBuilder::BlockChainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockChainConfiguration& config,
			bool isChained)
			: m_pAccounts(&accounts)
			, m_pStateHashCalculator(&stateHashCalculator)
			, m_blockTimeInterval(60'000)
			, m_config(config) {
		if (isChained)
			return;

		// seed the state hash calculator with the nemesis state
		const auto& dataDirectory = m_pStateHashCalculator->dataDirectory();
		if (dataDirectory.empty()) {
			mocks::MockMemoryBlockStorage storage;
			m_pParentBlockElement = storage.loadBlockElement(Height(1));

			// extend lifetime of nemesis block (referenced by m_pParentBlockElement) beyond lifetime of storage
			// (FileBlockStorage automatically extends block lifetime)
			m_pNemesisBlock = storage.loadBlock(Height(1));
		} else {
			io::FileBlockStorage storage(dataDirectory);
			m_pParentBlockElement = storage.loadBlockElement(Height(1));
		}

		m_pStateHashCalculator->execute(m_pParentBlockElement->Block);
	}

	void BlockChainBuilder::addTransfer(size_t senderId, size_t recipientId, Amount transferAmount) {
		m_transferDescriptors.push_back(TransferDescriptor{ senderId, recipientId, transferAmount });
	}

	void BlockChainBuilder::addNamespace(size_t ownerId, const std::string& name, BlockDuration duration) {
		m_namespaceDescriptors.push_back(NamespaceDescriptor{ ownerId, name, duration });
	}

	void BlockChainBuilder::setBlockTimeInterval(Timestamp blockTimeInterval) {
		m_blockTimeInterval = blockTimeInterval;
	}

	BlockChainBuilder BlockChainBuilder::createChainedBuilder() {
		return createChainedBuilder(*m_pStateHashCalculator);
	}

	BlockChainBuilder BlockChainBuilder::createChainedBuilder(StateHashCalculator& stateHashCalculator) {
		auto builder = BlockChainBuilder(*m_pAccounts, stateHashCalculator, m_config, true);
		builder.m_pParentBlockElement = m_pTailBlockElement;
		builder.m_difficulties = m_difficulties;
		return builder;
	}

	std::unique_ptr<model::Block> BlockChainBuilder::asSingleBlock() {
		model::PreviousBlockContext context(*m_pParentBlockElement);
		pushDifficulty(m_pParentBlockElement->Block);

		model::Transactions transactions;
		auto blockTimestamp = context.Timestamp + m_blockTimeInterval;
		for (const auto& descriptor : m_transferDescriptors)
			transactions.push_back(createTransfer(descriptor, blockTimestamp));

		for (const auto& descriptor : m_namespaceDescriptors)
			transactions.push_back(createRegisterNamespace(descriptor, blockTimestamp));

		auto pBlock = createBlock(context, blockTimestamp, transactions);
		m_pTailBlockElement = ToSharedBlockElement(context.GenerationHash, *pBlock);
		return pBlock;
	}

	BlockChainBuilder::Blocks BlockChainBuilder::asBlockChain() {
		auto pParentBlockElement = m_pParentBlockElement;

		Blocks blocks;
		for (const auto& descriptor : m_transferDescriptors) {
			model::PreviousBlockContext context(*pParentBlockElement);
			pushDifficulty(pParentBlockElement->Block);

			auto blockTimestamp = context.Timestamp + m_blockTimeInterval;
			auto pBlock = createBlock(context, blockTimestamp, { createTransfer(descriptor, blockTimestamp) });
			pParentBlockElement = ToSharedBlockElement(context.GenerationHash, *pBlock);
			blocks.push_back(std::move(pBlock));
		}

		m_pTailBlockElement = pParentBlockElement;
		return blocks;
	}

	void BlockChainBuilder::pushDifficulty(const model::Block& block) {
		m_difficulties.insert(state::BlockDifficultyInfo(block.Height, block.Timestamp, block.Difficulty));

		if (m_difficulties.size() > m_config.MaxDifficultyBlocks)
			m_difficulties.erase(m_difficulties.cbegin());
	}

	std::unique_ptr<model::Transaction> BlockChainBuilder::createTransfer(const TransferDescriptor& descriptor, Timestamp deadline) {
		const auto& senderKeyPair = m_pAccounts->getKeyPair(descriptor.SenderId);
		const auto& recipientKeyPair = m_pAccounts->getKeyPair(descriptor.RecipientId);

		auto pTransaction = CreateTransferTransaction(senderKeyPair, recipientKeyPair.publicKey(), descriptor.Amount);
		return SignWithDeadline(std::move(pTransaction), senderKeyPair, deadline);
	}

	std::unique_ptr<model::Transaction> BlockChainBuilder::createRegisterNamespace(
			const NamespaceDescriptor& descriptor,
			Timestamp deadline) {
		const auto& ownerKeyPair = m_pAccounts->getKeyPair(descriptor.OwnerId);

		auto pTransaction = CreateRegisterRootNamespaceTransaction(ownerKeyPair, descriptor.Name, descriptor.Duration);
		return SignWithDeadline(std::move(pTransaction), ownerKeyPair, deadline);
	}

	std::unique_ptr<model::Block> BlockChainBuilder::createBlock(
			const model::PreviousBlockContext& context,
			Timestamp timestamp,
			const model::Transactions& transactions) {
		auto difficulty = chain::CalculateDifficulty(cache::DifficultyInfoRange(m_difficulties.cbegin(), m_difficulties.cend()), m_config);

		auto signer = findSigner(context, timestamp, difficulty);
		auto pBlock = model::CreateBlock(context, model::NetworkIdentifier::Mijin_Test, signer.publicKey(), transactions);
		pBlock->Timestamp = timestamp;
		pBlock->Difficulty = difficulty;

		m_pStateHashCalculator->updateStateHash(*pBlock);
		SignBlock(signer, *pBlock);
		return pBlock;
	}

	crypto::KeyPair BlockChainBuilder::findSigner(const model::PreviousBlockContext& context, Timestamp timestamp, Difficulty difficulty) {
		chain::BlockHitPredicate hitPredicate(m_config, [](const auto&, auto) {
			// to simplfy tests, just return a constant importance
			return Importance(8'999'999'998 / CountOf(Mijin_Test_Private_Keys));
		});

		auto i = 0u;
		for (const auto* pPrivateKeyString : Mijin_Test_Private_Keys) {
			// skip first test account because it is used to fund other accounts
			if (0u == i++)
				continue;

			auto keyPair = crypto::KeyPair::FromString(pPrivateKeyString);

			chain::BlockHitContext blockHitContext;
			blockHitContext.GenerationHash = model::CalculateGenerationHash(context.GenerationHash, keyPair.publicKey());
			blockHitContext.ElapsedTime = utils::TimeSpan::FromDifference(timestamp, context.Timestamp);
			blockHitContext.Signer = keyPair.publicKey();
			blockHitContext.Difficulty = difficulty;
			blockHitContext.Height = context.BlockHeight + Height(1);

			if (hitPredicate(blockHitContext))
				return keyPair;
		}

		CATAPULT_THROW_RUNTIME_ERROR("no eligible harvesting accounts were found");
	}

	std::shared_ptr<const model::BlockElement> BlockChainBuilder::ToSharedBlockElement(
			const Hash256& parentGenerationHash,
			const model::Block& block) {
		auto pBlockElement = std::make_shared<model::BlockElement>(BlockToBlockElement(block));
		pBlockElement->GenerationHash = model::CalculateGenerationHash(parentGenerationHash, block.Signer);
		return pBlockElement;
	}

	std::unique_ptr<model::Transaction> BlockChainBuilder::SignWithDeadline(
			std::unique_ptr<model::Transaction>&& pTransaction,
			const crypto::KeyPair& signerKeyPair,
			Timestamp deadline) {
		pTransaction->Deadline = deadline;
		pTransaction->Fee = Amount(0);
		extensions::SignTransaction(signerKeyPair, *pTransaction);
		return std::move(pTransaction);
	}
}}
