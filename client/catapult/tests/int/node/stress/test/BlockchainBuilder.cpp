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

#include "BlockchainBuilder.h"
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/chain/BlockDifficultyScorer.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/io/FileBlockStorage.h"
#include "catapult/model/Address.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/preprocessor.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/test/nodeps/TestNetworkConstants.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Testnet;
	}

	BlockchainBuilder::BlockchainBuilder(const Accounts& accounts, StateHashCalculator& stateHashCalculator)
			: BlockchainBuilder(accounts, stateHashCalculator, CreatePrototypicalBlockchainConfiguration()) {
	}

	BlockchainBuilder::BlockchainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockchainConfiguration& config)
			: BlockchainBuilder(accounts, stateHashCalculator, config, stateHashCalculator.dataDirectory()) {
	}

	BlockchainBuilder::BlockchainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockchainConfiguration& config,
			const std::string& resourcesPath)
			: BlockchainBuilder(accounts, stateHashCalculator, config, resourcesPath, false) {
	}

	BlockchainBuilder::BlockchainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockchainConfiguration& config,
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
			CATAPULT_LOG(debug) << "initializing BlockchainBuilder from memory";
			mocks::MockMemoryBlockStorage storage;
			m_pParentBlockElement = storage.loadBlockElement(Height(1));

			// extend lifetime of nemesis block (referenced by m_pParentBlockElement) beyond lifetime of storage
			// (FileBlockStorage automatically extends block lifetime)
			m_pNemesisBlock = storage.loadBlock(Height(1));
		} else {
			CATAPULT_LOG(debug) << "initializing BlockchainBuilder from resources path: " << resourcesPath;
			io::FileBlockStorage storage(resourcesPath, File_Database_Batch_Size);
			m_pParentBlockElement = storage.loadBlockElement(Height(1));
		}

		m_previousImportanceBlockHash = m_pParentBlockElement->EntityHash;
		m_pStateHashCalculator->execute(m_pParentBlockElement->Block);
	}

	void BlockchainBuilder::setBlockTimeInterval(utils::TimeSpan blockTimeInterval) {
		m_blockTimeInterval = blockTimeInterval;
	}

	void BlockchainBuilder::setBlockReceiptsHashCalculator(const BlockReceiptsHashCalculator& blockReceiptsHashCalculator) {
		m_blockReceiptsHashCalculator = blockReceiptsHashCalculator;
	}

	BlockchainBuilder BlockchainBuilder::createChainedBuilder() {
		return createChainedBuilder(*m_pStateHashCalculator);
	}

	BlockchainBuilder BlockchainBuilder::createChainedBuilder(StateHashCalculator& stateHashCalculator) const {
		// resources directory is not used when creating chained builder
		auto builder = BlockchainBuilder(*m_pAccounts, stateHashCalculator, m_config, "", true);
		builder.m_pTailBlockElement = m_pTailBlockElement;
		builder.m_pParentBlockElement = m_pTailBlockElement;
		builder.m_statistics = m_statistics;
		builder.m_previousImportanceBlockHash = m_previousImportanceBlockHash;
		return builder;
	}

	BlockchainBuilder BlockchainBuilder::createChainedBuilder(StateHashCalculator& stateHashCalculator, const model::Block& block) const {
		// resources directory is not used when creating chained builder
		auto builder = BlockchainBuilder(*m_pAccounts, stateHashCalculator, m_config, "", true);
		builder.m_pTailBlockElement = ToSharedBlockElement(block);
		builder.m_pParentBlockElement = builder.m_pTailBlockElement;
		builder.m_statistics = m_statistics;
		builder.m_previousImportanceBlockHash = m_previousImportanceBlockHash;
		return builder;
	}

	std::unique_ptr<model::Block> BlockchainBuilder::asSingleBlock(const TransactionsGenerator& transactionsGenerator) {
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

	BlockchainBuilder::Blocks BlockchainBuilder::asBlockchain(const TransactionsGenerator& transactionsGenerator) {
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

	void BlockchainBuilder::pushDifficulty(const model::Block& block) {
		m_statistics.insert(state::BlockStatistic(block));

		if (m_statistics.size() > m_config.MaxDifficultyBlocks)
			m_statistics.erase(m_statistics.cbegin());
	}

	std::unique_ptr<model::Block> BlockchainBuilder::createBlock(
			const model::PreviousBlockContext& context,
			Timestamp timestamp,
			const model::Transactions& transactions) {
		auto difficulty = chain::CalculateDifficulty(cache::BlockStatisticRange(m_statistics.cbegin(), m_statistics.cend()), m_config);

		auto entityType = model::CalculateBlockTypeFromHeight(context.BlockHeight + Height(1), m_config.ImportanceGrouping);
		auto signerKeyPair = findBlockSigner(context, timestamp, difficulty);
		auto pBlock = model::CreateBlock(entityType, context, Network_Identifier, signerKeyPair.publicKey(), transactions);
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

		auto isImportanceBlock = model::Entity_Type_Block_Importance == entityType;
		if (isImportanceBlock) {
			auto& blockFooter = model::GetBlockFooter<model::ImportanceBlockFooter>(*pBlock);
			blockFooter.HarvestingEligibleAccountsCount = CountOf(Test_Network_Vrf_Private_Keys);
			blockFooter.PreviousImportanceBlockHash = m_previousImportanceBlockHash;
		}

		extensions::BlockExtensions(GetNemesisGenerationHashSeed()).signFullBlock(signerKeyPair, *pBlock);

		if (isImportanceBlock)
			m_previousImportanceBlockHash = model::CalculateHash(*pBlock);

		return pBlock;
	}

	crypto::KeyPair BlockchainBuilder::findBlockSigner(
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

	std::shared_ptr<const model::BlockElement> BlockchainBuilder::ToSharedBlockElement(const model::Block& block) {
		return std::make_shared<model::BlockElement>(BlockToBlockElement(block, GenerationHashSeed()));
	}
}}
