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

#include "Harvester.h"
#include "catapult/cache_core/ImportanceView.h"
#include "catapult/chain/BlockDifficultyScorer.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace harvesting {

	namespace {
		struct NextBlockContext {
		public:
			NextBlockContext(const model::BlockElement& parentBlockElement, Timestamp nextTimestamp)
					: ParentBlock(parentBlockElement.Block)
					, ParentContext(parentBlockElement)
					, Timestamp(nextTimestamp)
					, Height(ParentBlock.Height + catapult::Height(1))
					, BlockTime(utils::TimeSpan::FromDifference(Timestamp, ParentBlock.Timestamp))
			{}

		public:
			const model::Block& ParentBlock;
			model::PreviousBlockContext ParentContext;
			catapult::Timestamp Timestamp;
			catapult::Height Height;
			utils::TimeSpan BlockTime;
			catapult::Difficulty Difficulty;

		public:
			bool tryCalculateDifficulty(const cache::BlockStatisticCache& cache, const model::BlockChainConfiguration& config) {
				return chain::TryCalculateDifficulty(cache, ParentBlock.Height, config, Difficulty);
			}
		};

		std::unique_ptr<model::Block> CreateUnsignedBlockHeader(
				const NextBlockContext& context,
				model::EntityType blockType,
				uint8_t version,
				model::NetworkIdentifier networkIdentifier,
				const Key& signer,
				const Address& beneficiary) {
			auto pBlock = model::CreateBlock(blockType, context.ParentContext, networkIdentifier, signer, {});
			pBlock->Version = version;
			pBlock->Difficulty = context.Difficulty;
			pBlock->Timestamp = context.Timestamp;
			if (Address() != beneficiary)
				pBlock->BeneficiaryAddress = beneficiary;

			return pBlock;
		}

		void AddGenerationHashProof(model::Block& block, const crypto::VrfProof& vrfProof) {
			block.GenerationHashProof = { vrfProof.Gamma, vrfProof.VerificationHash, vrfProof.Scalar };
		}

		model::EntityType HeightToBlockType(const model::BlockChainConfiguration& config, Height height) {
			if (height <= config.ForkHeights.ImportanceBlock)
				return model::Entity_Type_Block_Normal;

			return model::CalculateBlockTypeFromHeight(height, config.ImportanceGrouping);
		}

		uint8_t HeightToVersion(const model::BlockChainConfiguration& config, Height height) {
			return height <= config.ForkHeights.ImportanceBlock ? 1 : model::Block::Current_Version;
		}
	}

	Harvester::Harvester(
			const cache::CatapultCache& cache,
			const model::BlockChainConfiguration& config,
			const Address& beneficiary,
			const UnlockedAccounts& unlockedAccounts,
			const BlockGenerator& blockGenerator)
			: m_cache(cache)
			, m_config(config)
			, m_beneficiary(beneficiary)
			, m_unlockedAccounts(unlockedAccounts)
			, m_blockGenerator(blockGenerator)
	{}

	std::unique_ptr<model::Block> Harvester::harvest(const model::BlockElement& lastBlockElement, Timestamp timestamp) {
		NextBlockContext context(lastBlockElement, timestamp);
		if (!context.tryCalculateDifficulty(m_cache.sub<cache::BlockStatisticCache>(), m_config)) {
			CATAPULT_LOG(debug) << "skipping harvest attempt due to error calculating difficulty";
			return nullptr;
		}

		chain::BlockHitContext hitContext;
		hitContext.ElapsedTime = context.BlockTime;
		hitContext.Difficulty = context.Difficulty;
		hitContext.Height = context.Height;

		const auto& accountStateCache = m_cache.sub<cache::AccountStateCache>();
		chain::BlockHitPredicate hitPredicate(m_config, [&accountStateCache](const auto& key, auto height) {
			auto lockedCacheView = accountStateCache.createView();
			cache::ReadOnlyAccountStateCache readOnlyCache(*lockedCacheView);
			cache::ImportanceView view(readOnlyCache);
			return view.getAccountImportanceOrDefault(key, height);
		});

		auto unlockedAccountsView = m_unlockedAccounts.view();
		const crypto::KeyPair* pHarvesterKeyPair = nullptr;
		crypto::VrfProof vrfProof;

		unlockedAccountsView.forEach([&context, &hitContext, &hitPredicate, &pHarvesterKeyPair, &vrfProof](const auto& descriptor) {
			hitContext.Signer = descriptor.signingKeyPair().publicKey();

			vrfProof = crypto::GenerateVrfProof(context.ParentContext.GenerationHash, descriptor.vrfKeyPair());
			hitContext.GenerationHash = model::CalculateGenerationHash(vrfProof.Gamma);
			if (hitPredicate(hitContext)) {
				pHarvesterKeyPair = &descriptor.signingKeyPair();
				return false;
			}

			return true;
		});

		if (!pHarvesterKeyPair)
			return nullptr;

		utils::StackLogger stackLogger("generating candidate block", utils::LogLevel::debug);
		auto pBlockHeader = CreateUnsignedBlockHeader(
				context,
				HeightToBlockType(m_config, context.Height),
				HeightToVersion(m_config, context.Height),
				m_config.Network.Identifier,
				pHarvesterKeyPair->publicKey(),
				m_beneficiary);

		AddGenerationHashProof(*pBlockHeader, vrfProof);
		auto pBlock = m_blockGenerator(*pBlockHeader, m_config.MaxTransactionsPerBlock);
		if (pBlock)
			SignBlockHeader(*pHarvesterKeyPair, *pBlock);

		return pBlock;
	}
}}
