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

#include "FinalizationMessageFactory.h"
#include "finalization/src/FinalizationConfiguration.h"
#include "finalization/src/io/PrevoteChainStorage.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "finalization/src/model/VotingSet.h"
#include "catapult/crypto_voting/AggregateBmPrivateKeyTree.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/HeightGrouping.h"

namespace catapult { namespace chain {

	namespace {
		// region utils

		uint64_t Clamp(uint64_t value, uint16_t multiple, uint16_t adjustment) {
			return 0 == value % multiple ? value : ((value / multiple + adjustment) * multiple);
		}

		model::HashRange ToHashRange(const Hash256& hash) {
			return model::HashRange::CopyFixed(reinterpret_cast<const uint8_t*>(&hash), 1);
		}

		// endregion

		// region DefaultFinalizationMessageFactory

		class DefaultFinalizationMessageFactory : public FinalizationMessageFactory {
		public:
			DefaultFinalizationMessageFactory(
					const finalization::FinalizationConfiguration& config,
					const io::BlockStorageCache& blockStorage,
					const io::ProofStorageCache& proofStorage,
					const PrevoteChainDescriptorConsumer& prevoteChainDescriptorConsumer,
					crypto::AggregateBmPrivateKeyTree&& bmPrivateKeyTree)
					: m_config(config)
					, m_blockStorage(blockStorage)
					, m_proofStorage(proofStorage)
					, m_prevoteChainDescriptorConsumer(prevoteChainDescriptorConsumer)
					, m_bmPrivateKeyTree(std::move(bmPrivateKeyTree))
			{}

		public:
			std::unique_ptr<model::FinalizationMessage> createPrevote(const model::FinalizationRound& round) override {
				auto finalizationStatistics = m_proofStorage.view().statistics();
				auto hashRange = loadPrevoteHashChain(round, finalizationStatistics.Height);
				if (hashRange.empty())
					hashRange = ToHashRange(finalizationStatistics.Hash);

				auto stepIdentifier = model::StepIdentifier{ round.Epoch, round.Point, model::FinalizationStage::Prevote };
				return model::PrepareMessage(m_bmPrivateKeyTree, stepIdentifier, finalizationStatistics.Height, hashRange);
			}

			std::unique_ptr<model::FinalizationMessage> createPrecommit(
					const model::FinalizationRound& round,
					Height height,
					const Hash256& hash) override {
				auto hashRange = ToHashRange(hash);

				auto stepIdentifier = model::StepIdentifier{ round.Epoch, round.Point, model::FinalizationStage::Precommit };
				return model::PrepareMessage(m_bmPrivateKeyTree, stepIdentifier, height, hashRange);
			}

		private:
			model::HashRange loadPrevoteHashChain(const model::FinalizationRound& round, Height startHeight) const {
				auto view = m_blockStorage.view();
				auto chainHeight = view.chainHeight().unwrap();
				auto multiple = m_config.PrevoteBlocksMultiple;
				auto maxPrevoteHashHeight = chainHeight >= multiple ? Height(chainHeight - multiple) : Height(chainHeight);

				auto maxVotingSetHeight = model::CalculateVotingSetEndHeight(round.Epoch, m_config.VotingSetGrouping);
				if (maxPrevoteHashHeight > maxVotingSetHeight)
					maxPrevoteHashHeight = maxVotingSetHeight;

				auto clampedChainHeight = Height(Clamp(maxPrevoteHashHeight.unwrap(), m_config.PrevoteBlocksMultiple, 0));

				auto numHashes = clampedChainHeight > startHeight
						? (clampedChainHeight - startHeight).unwrap() + 1
						: 1;

				if (numHashes > m_config.MaxHashesPerPoint)
					numHashes -= Clamp(numHashes - m_config.MaxHashesPerPoint, m_config.PrevoteBlocksMultiple, 1);

				m_prevoteChainDescriptorConsumer(view, { round, startHeight, numHashes });
				return view.loadHashesFrom(startHeight, numHashes);
			}

		private:
			finalization::FinalizationConfiguration m_config;
			const io::BlockStorageCache& m_blockStorage;
			const io::ProofStorageCache& m_proofStorage;
			PrevoteChainDescriptorConsumer m_prevoteChainDescriptorConsumer;
			crypto::AggregateBmPrivateKeyTree m_bmPrivateKeyTree;
		};

		// endregion
	}

	std::unique_ptr<FinalizationMessageFactory> CreateFinalizationMessageFactory(
			const finalization::FinalizationConfiguration& config,
			const io::BlockStorageCache& blockStorage,
			const io::ProofStorageCache& proofStorage,
			const PrevoteChainDescriptorConsumer& prevoteChainDescriptorConsumer,
			crypto::AggregateBmPrivateKeyTree&& bmPrivateKeyTree) {
		return std::make_unique<DefaultFinalizationMessageFactory>(
				config,
				blockStorage,
				proofStorage,
				prevoteChainDescriptorConsumer,
				std::move(bmPrivateKeyTree));
	}
}}
