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
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/chain/BlockDifficultyScorer.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/io/FileBlockStorage.h"
#include "catapult/model/Address.h"
#include "catapult/preprocessor.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/test/nodeps/TestNetworkConstants.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
	}

	BlockChainBuilder::BlockChainBuilder(const Accounts& accounts, StateHashCalculator& stateHashCalculator)
			: BlockChainBuilder(accounts, stateHashCalculator, CreatePrototypicalBlockChainConfiguration())
	{}

	BlockChainBuilder::BlockChainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockChainConfiguration& config)
			: BlockChainBuilder(accounts, stateHashCalculator, config, stateHashCalculator.dataDirectory())
	{}

	BlockChainBuilder::BlockChainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockChainConfiguration& config,
			const std::string& resourcesPath)
			: BlockChainBuilder(accounts, stateHashCalculator, config, resourcesPath, false)
	{}

	BlockChainBuilder::BlockChainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockChainConfiguration& config,
			const std::string& resourcesPath,
			bool isChained)
			: m_pAccounts(&accounts)
			, m_pStateHashCalculator(&stateHashCalculator)
			, m_blockTimeInterval(utils::TimeSpan::FromSeconds(60))
			, m_blockReceiptsHashCalculator([](const auto&) { return Hash256(); })
			, m_config(config) {
		if (isChained)
			return;

		// seed the state hash calculator with the nemesis state
		if (resourcesPath.empty()) {
			CATAPULT_LOG(debug) << "initializing BlockChainBuilder from memory";
			mocks::MockMemoryBlockStorage storage;
			m_pParentBlockElement = storage.loadBlockElement(Height(1));

			// extend lifetime of nemesis block (referenced by m_pParentBlockElement) beyond lifetime of storage
			// (FileBlockStorage automatically extends block lifetime)
			m_pNemesisBlock = storage.loadBlock(Height(1));
		} else {
			CATAPULT_LOG(debug) << "initializing BlockChainBuilder from resources path: " << resourcesPath;
			io::FileBlockStorage storage(resourcesPath);
			m_pParentBlockElement = storage.loadBlockElement(Height(1));
		}

		m_pStateHashCalculator->execute(m_pParentBlockElement->Block);
	}

	void BlockChainBuilder::setBlockTimeInterval(utils::TimeSpan blockTimeInterval) {
		m_blockTimeInterval = blockTimeInterval;
	}

	void BlockChainBuilder::setBlockReceiptsHashCalculator(const BlockReceiptsHashCalculator& blockReceiptsHashCalculator) {
		m_blockReceiptsHashCalculator = blockReceiptsHashCalculator;
	}

	BlockChainBuilder BlockChainBuilder::createChainedBuilder() {
		return createChainedBuilder(*m_pStateHashCalculator);
	}

	BlockChainBuilder BlockChainBuilder::createChainedBuilder(StateHashCalculator& stateHashCalculator) const {
		// resources directory is not used when creating chained builder
		auto builder = BlockChainBuilder(*m_pAccounts, stateHashCalculator, m_config, "", true);
		builder.m_pTailBlockElement = m_pTailBlockElement;
		builder.m_pParentBlockElement = m_pTailBlockElement;
		builder.m_statistics = m_statistics;
		return builder;
	}

	BlockChainBuilder BlockChainBuilder::createChainedBuilder(StateHashCalculator& stateHashCalculator, const model::Block& block) const {
		// resources directory is not used when creating chained builder
		auto builder = BlockChainBuilder(*m_pAccounts, stateHashCalculator, m_config, "", true);
		builder.m_pTailBlockElement = ToSharedBlockElement(block);
		builder.m_pParentBlockElement = builder.m_pTailBlockElement;
		builder.m_statistics = m_statistics;
		return builder;
	}

	std::unique_ptr<model::Block> BlockChainBuilder::asSingleBlock(const TransactionsGenerator& transactionsGenerator) {
		model::PreviousBlockContext context(*m_pParentBlockElement);
		pushDifficulty(m_pParentBlockElement->Block);

		model::Transactions transactions;
		auto blockTimestamp = context.Timestamp + m_blockTimeInterval;
		for (auto i = 0u; i < transactionsGenerator.size(); ++i)
			transactions.push_back(transactionsGenerator.generateAt(i, blockTimestamp));

		auto pBlock = createBlock(context, blockTimestamp, transactions);
		m_pTailBlockElement = ToSharedBlockElement(*pBlock);
		m_pParentBlockElement = m_pTailBlockElement;
		return pBlock;
	}

	BlockChainBuilder::Blocks BlockChainBuilder::asBlockChain(const TransactionsGenerator& transactionsGenerator) {
		Blocks blocks;
		for (auto i = 0u; i < transactionsGenerator.size(); ++i) {
			model::PreviousBlockContext context(*m_pParentBlockElement);
			pushDifficulty(m_pParentBlockElement->Block);

			auto blockTimestamp = context.Timestamp + m_blockTimeInterval;
			auto pBlock = createBlock(context, blockTimestamp, { transactionsGenerator.generateAt(i, blockTimestamp) });
			m_pParentBlockElement = ToSharedBlockElement(*pBlock);
			blocks.push_back(std::move(pBlock));
		}

		m_pTailBlockElement = m_pParentBlockElement;
		return blocks;
	}

	void BlockChainBuilder::pushDifficulty(const model::Block& block) {
		m_statistics.insert(state::BlockStatistic(block));

		if (m_statistics.size() > m_config.MaxDifficultyBlocks)
			m_statistics.erase(m_statistics.cbegin());
	}

	std::unique_ptr<model::Block> BlockChainBuilder::createBlock(
			const model::PreviousBlockContext& context,
			Timestamp timestamp,
			const model::Transactions& transactions) {
		auto difficulty = chain::CalculateDifficulty(cache::BlockStatisticRange(m_statistics.cbegin(), m_statistics.cend()), m_config);

		auto signerKeyPair = findBlockSigner(context, timestamp, difficulty);
		auto pBlock = model::CreateBlock(context, Network_Identifier, signerKeyPair.publicKey(), transactions);
		pBlock->Timestamp = timestamp;
		pBlock->Difficulty = difficulty;

		// beneficiary must be a fixed low value account so that block rollback and reapply tests do not change AccountStateCache
		// by changing an account's beneficiary count
		pBlock->BeneficiaryAddress = model::PublicKeyToAddress({ { 1 } }, Network_Identifier);

		pBlock->ReceiptsHash = m_blockReceiptsHashCalculator(*pBlock);
		m_pStateHashCalculator->updateStateHash(*pBlock);

		auto vrfKeyPair = LookupVrfKeyPair(signerKeyPair.publicKey());
		auto vrfProof = crypto::GenerateVrfProof(context.GenerationHash, vrfKeyPair);
		pBlock->GenerationHashProof = { vrfProof.Gamma, vrfProof.VerificationHash, vrfProof.Scalar };

		extensions::BlockExtensions(GetNemesisGenerationHashSeed()).signFullBlock(signerKeyPair, *pBlock);
		return pBlock;
	}

	crypto::KeyPair BlockChainBuilder::findBlockSigner(
			const model::PreviousBlockContext& context,
			Timestamp timestamp,
			Difficulty difficulty) {
		chain::BlockHitPredicate hitPredicate(m_config, [](const auto& publicKey, auto) {
			// to simplfy tests, just return nemesis importance
			return GetNemesisImportance(publicKey);
		});

		auto i = 0u;
		for (const auto* pPrivateKeyString : Test_Network_Private_Keys) {
			// skip first test account because it is used to fund other accounts
			if (0u == i++)
				continue;

			auto signerKeyPair = crypto::KeyPair::FromString(pPrivateKeyString);
			auto vrfKeyPair = LookupVrfKeyPair(signerKeyPair.publicKey());
			auto vrfProof = crypto::GenerateVrfProof(context.GenerationHash, vrfKeyPair);

			chain::BlockHitContext blockHitContext;
			blockHitContext.GenerationHash = model::CalculateGenerationHash(vrfProof.Gamma);
			blockHitContext.ElapsedTime = utils::TimeSpan::FromDifference(timestamp, context.Timestamp);
			blockHitContext.Signer = signerKeyPair.publicKey();
			blockHitContext.Difficulty = difficulty;
			blockHitContext.Height = context.BlockHeight + Height(1);

			if (hitPredicate(blockHitContext))
				return signerKeyPair;
		}

		CATAPULT_THROW_RUNTIME_ERROR("no eligible harvesting accounts were found");
	}

	std::shared_ptr<const model::BlockElement> BlockChainBuilder::ToSharedBlockElement(const model::Block& block) {
		return std::make_shared<model::BlockElement>(BlockToBlockElement(block, GenerationHashSeed()));
	}
}}
