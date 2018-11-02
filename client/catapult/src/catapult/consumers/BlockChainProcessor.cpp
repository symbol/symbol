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

#include "BlockChainProcessor.h"
#include "InputUtils.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/chain/ChainResults.h"
#include "catapult/chain/ChainUtils.h"
#include "catapult/model/BlockUtils.h"

using namespace catapult::validators;

namespace catapult { namespace consumers {

	namespace {
		model::WeakEntityInfos ExtractEntityInfos(const model::BlockElement& element) {
			model::WeakEntityInfos entityInfos;
			model::ExtractEntityInfos(element, entityInfos);
			return entityInfos;
		}

		bool IsLinked(const WeakBlockInfo& parentBlockInfo, const BlockElements& elements) {
			return chain::IsChainLink(parentBlockInfo.entity(), parentBlockInfo.hash(), elements[0].Block);
		}

		void LogCacheStateHashInformation(Height height, const cache::StateHashInfo& stateHashInfo) {
			std::ostringstream formattedSubCacheMerkleRoots;
			for (const auto& subCacheMerkleRoot : stateHashInfo.SubCacheMerkleRoots)
				formattedSubCacheMerkleRoots << std::endl << " + " << utils::HexFormat(subCacheMerkleRoot);

			CATAPULT_LOG(debug)
					<< "cache state hash (" << stateHashInfo.SubCacheMerkleRoots.size() << " components) at height " << height
					<< std::endl << utils::HexFormat(stateHashInfo.StateHash)
					<< formattedSubCacheMerkleRoots.str();
		}

		class DefaultBlockChainProcessor {
		public:
			DefaultBlockChainProcessor(
					const BlockHitPredicateFactory& blockHitPredicateFactory,
					const chain::BatchEntityProcessor& batchEntityProcessor)
					: m_blockHitPredicateFactory(blockHitPredicateFactory)
					, m_batchEntityProcessor(batchEntityProcessor)
			{}

		public:
			ValidationResult operator()(
					const WeakBlockInfo& parentBlockInfo,
					BlockElements& elements,
					const observers::ObserverState& state) const {
				if (elements.empty())
					return ValidationResult::Neutral;

				if (!IsLinked(parentBlockInfo, elements))
					return chain::Failure_Chain_Unlinked;

				auto readOnlyCache = state.Cache.toReadOnly();
				auto blockHitPredicate = m_blockHitPredicateFactory(readOnlyCache);

				const auto* pParent = &parentBlockInfo.entity();
				const auto* pParentGenerationHash = &parentBlockInfo.generationHash();

				// initial cache state will be either last cache state or unwound cache state
				LogCacheStateHashInformation(pParent->Height, state.Cache.calculateStateHash(pParent->Height));

				for (auto& element : elements) {
					const auto& block = element.Block;
					element.GenerationHash = model::CalculateGenerationHash(*pParentGenerationHash, block.Signer);
					if (!blockHitPredicate(*pParent, block, element.GenerationHash)) {
						CATAPULT_LOG(warning) << "block " << block.Height << " failed hit";
						return chain::Failure_Chain_Block_Not_Hit;
					}

					auto result = m_batchEntityProcessor(block.Height, block.Timestamp, ExtractEntityInfos(element), state);
					if (!IsValidationResultSuccess(result)) {
						CATAPULT_LOG(warning) << "batch processing of block " << block.Height << " failed with " << result;
						return result;
					}

					auto cacheStateHashInfo = state.Cache.calculateStateHash(block.Height);
					LogCacheStateHashInformation(block.Height, cacheStateHashInfo);

					if (block.StateHash != cacheStateHashInfo.StateHash) {
						CATAPULT_LOG(warning)
								<< "block state hash (" << utils::HexFormat(block.StateHash) << ") does not match "
								<< "cache state hash (" << utils::HexFormat(cacheStateHashInfo.StateHash) << ") "
								<< "at height " << block.Height;
						return chain::Failure_Chain_Block_Inconsistent_State_Hash;
					}

					element.SubCacheMerkleRoots = cacheStateHashInfo.SubCacheMerkleRoots;

					pParent = &block;
					pParentGenerationHash = &element.GenerationHash;
				}

				return ValidationResult::Success;
			}

		private:
			BlockHitPredicateFactory m_blockHitPredicateFactory;
			chain::BatchEntityProcessor m_batchEntityProcessor;
		};
	}

	BlockChainProcessor CreateBlockChainProcessor(
			const BlockHitPredicateFactory& blockHitPredicateFactory,
			const chain::BatchEntityProcessor& batchEntityProcessor) {
		return DefaultBlockChainProcessor(blockHitPredicateFactory, batchEntityProcessor);
	}
}}
