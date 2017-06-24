#include "Harvester.h"
#include "BlockDifficultyScorer.h"
#include "BlockScorer.h"
#include "catapult/cache/ImportanceView.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/model/BlockUtils.h"

using namespace catapult::model;

namespace catapult { namespace chain {

	namespace {
		struct NextBlockContext {
		public:
			explicit NextBlockContext(const BlockElement& parentBlockElement, Timestamp nextTimestamp)
					: ParentBlock(parentBlockElement.Block)
					, ParentContext(parentBlockElement)
					, Timestamp(nextTimestamp)
					, Height(ParentBlock.Height + catapult::Height(1))
					, BlockTime(utils::TimeSpan::FromDifference(Timestamp, ParentBlock.Timestamp))
			{}

		public:
			const Block& ParentBlock;
			PreviousBlockContext ParentContext;
			catapult::Timestamp Timestamp;
			catapult::Height Height;
			utils::TimeSpan BlockTime;
			catapult::Difficulty Difficulty;

		public:
			bool tryCalculateDifficulty(const cache::BlockDifficultyCache& cache, const BlockChainConfiguration& config) {
				return TryCalculateDifficulty(cache, ParentBlock.Height, config, Difficulty);
			}
		};

		auto CreateBlock(
				const NextBlockContext& context,
				NetworkIdentifier networkIdentifier,
				const crypto::KeyPair& keyPair,
				const TransactionsInfo& info) {
			auto pBlock = model::CreateBlock(context.ParentContext, networkIdentifier, keyPair.publicKey(), info.Transactions);
			pBlock->Difficulty = context.Difficulty;
			pBlock->Timestamp = context.Timestamp;
			pBlock->BlockTransactionsHash = info.TransactionsHash;
			SignBlockHeader(keyPair, *pBlock);
			return pBlock;
		}
	}

	Harvester::Harvester(
			const cache::CatapultCache& cache,
			const BlockChainConfiguration& config,
			const UnlockedAccounts& unlockedAccounts,
			const TransactionsInfoSupplier& transactionsInfoSupplier)
			: m_cache(cache)
			, m_config(config)
			, m_unlockedAccounts(unlockedAccounts)
			, m_transactionsInfoSupplier(transactionsInfoSupplier)
	{}

	std::unique_ptr<Block> Harvester::harvest(const BlockElement& lastBlockElement, Timestamp timestamp) {
		NextBlockContext context(lastBlockElement, timestamp);
		if (!context.tryCalculateDifficulty(m_cache.sub<cache::BlockDifficultyCache>(), m_config)) {
			CATAPULT_LOG(warning) << "skipping harvest attempt due to error calculating difficulty";
			return nullptr;
		}

		BlockHitContext hitContext;
		hitContext.ElapsedTime = context.BlockTime;
		hitContext.Difficulty = context.Difficulty;
		hitContext.Height = context.Height;

		const auto& accountStateCache = m_cache.sub<cache::AccountStateCache>();
		BlockHitPredicate hitPredicate(m_config, [&accountStateCache](const auto& key, auto height) {
			auto lockedCacheView = accountStateCache.createView();
			cache::ReadOnlyAccountStateCache readOnlyCache(*lockedCacheView);
			cache::ImportanceView view(readOnlyCache);
			return view.getAccountImportanceOrDefault(key, height);
		});

		auto unlockedAccountsView = m_unlockedAccounts.view();
		const crypto::KeyPair* pHarvesterKeyPair = nullptr;
		for (const auto& keyPair : unlockedAccountsView) {
			hitContext.Signer = keyPair.publicKey();
			hitContext.GenerationHash = CalculateGenerationHash(context.ParentContext.GenerationHash, hitContext.Signer);

			if (hitPredicate(hitContext)) {
				pHarvesterKeyPair = &keyPair;
				break;
			}
		}

		if (!pHarvesterKeyPair)
			return nullptr;

		auto transactionsInfo = m_transactionsInfoSupplier(m_config.MaxTransactionsPerBlock);
		return CreateBlock(context, m_config.Network.Identifier, *pHarvesterKeyPair, transactionsInfo);
	}
}}
