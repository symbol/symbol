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
				formattedSubCacheMerkleRoots << std::endl << " + " << subCacheMerkleRoot;

			CATAPULT_LOG(debug)
					<< "cache state hash (" << stateHashInfo.SubCacheMerkleRoots.size() << " components) at height " << height
					<< std::endl << stateHashInfo.StateHash
					<< formattedSubCacheMerkleRoots.str();
		}

		class DefaultBlockChainProcessor {
		public:
			DefaultBlockChainProcessor(
					const BlockHitPredicateFactory& blockHitPredicateFactory,
					const chain::BatchEntityProcessor& batchEntityProcessor,
					ReceiptValidationMode receiptValidationMode)
					: m_blockHitPredicateFactory(blockHitPredicateFactory)
					, m_batchEntityProcessor(batchEntityProcessor)
					, m_receiptValidationMode(receiptValidationMode)
			{}

		public:
			ValidationResult operator()(
					const WeakBlockInfo& parentBlockInfo,
					BlockElements& elements,
					observers::ObserverState& state) const {
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
					// 1. check generation hash
					if (!CheckGenerationHash(element, *pParent, *pParentGenerationHash, blockHitPredicate))
						return chain::Failure_Chain_Block_Not_Hit;

					// 2. validate and observe block
					model::BlockStatementBuilder blockStatementBuilder;
					auto blockDependentState = createBlockDependentObserverState(state, blockStatementBuilder);

					const auto& block = element.Block;
					auto result = m_batchEntityProcessor(block.Height, block.Timestamp, ExtractEntityInfos(element), blockDependentState);
					if (!IsValidationResultSuccess(result)) {
						CATAPULT_LOG(warning) << "batch processing of block " << block.Height << " failed with " << result;
						return result;
					}

					// 3. check state hash
					if (!CheckStateHash(element, state.Cache))
						return chain::Failure_Chain_Block_Inconsistent_State_Hash;

					// 4. check receipts hash
					if (!CheckReceiptsHash(element, blockStatementBuilder, m_receiptValidationMode))
						return chain::Failure_Chain_Block_Inconsistent_Receipts_Hash;

					// 5. set next parent
					pParent = &block;
					pParentGenerationHash = &element.GenerationHash;
				}

				return ValidationResult::Success;
			}

		private:
			observers::ObserverState createBlockDependentObserverState(
					observers::ObserverState& state,
					model::BlockStatementBuilder& blockStatementBuilder) const {
				return ReceiptValidationMode::Disabled == m_receiptValidationMode
						? state
						: observers::ObserverState(state.Cache, state.State, blockStatementBuilder);
			}

		private:
			static bool CheckGenerationHash(
					model::BlockElement& element,
					const model::Block& parentBlock,
					const GenerationHash& parentGenerationHash,
					const BlockHitPredicate& blockHitPredicate) {
				const auto& block = element.Block;
				element.GenerationHash = model::CalculateGenerationHash(parentGenerationHash, block.Signer);
				if (!blockHitPredicate(parentBlock, block, element.GenerationHash)) {
					CATAPULT_LOG(warning) << "block " << block.Height << " failed hit";
					return false;
				}

				return true;
			}

			static bool CheckStateHash(model::BlockElement& element, cache::CatapultCacheDelta& cacheDelta) {
				const auto& block = element.Block;
				auto cacheStateHashInfo = cacheDelta.calculateStateHash(block.Height);
				LogCacheStateHashInformation(block.Height, cacheStateHashInfo);

				if (block.StateHash != cacheStateHashInfo.StateHash) {
					CATAPULT_LOG(warning)
							<< "block state hash (" << block.StateHash << ") does not match "
							<< "cache state hash (" << cacheStateHashInfo.StateHash << ") "
							<< "at height " << block.Height;
					return false;
				}

				element.SubCacheMerkleRoots = cacheStateHashInfo.SubCacheMerkleRoots;
				return true;
			}

			static bool CheckReceiptsHash(
					model::BlockElement& element,
					model::BlockStatementBuilder& blockStatementBuilder,
					ReceiptValidationMode receiptValidationMode) {
				const auto& block = element.Block;
				Hash256 blockReceiptsHash{};
				if (ReceiptValidationMode::Enabled == receiptValidationMode) {
					auto pBlockStatement = blockStatementBuilder.build();
					blockReceiptsHash = CalculateMerkleHash(*pBlockStatement);
					element.OptionalStatement = std::move(pBlockStatement);
				}

				if (block.BlockReceiptsHash != blockReceiptsHash) {
					CATAPULT_LOG(warning)
							<< "block receipts hash (" << block.BlockReceiptsHash << ") does not match "
							<< "calculated receipts hash (" << blockReceiptsHash << ") "
							<< "at height " << block.Height;
					return false;
				}

				return true;
			}

		private:
			BlockHitPredicateFactory m_blockHitPredicateFactory;
			chain::BatchEntityProcessor m_batchEntityProcessor;
			ReceiptValidationMode m_receiptValidationMode;
		};
	}

	BlockChainProcessor CreateBlockChainProcessor(
			const BlockHitPredicateFactory& blockHitPredicateFactory,
			const chain::BatchEntityProcessor& batchEntityProcessor,
			ReceiptValidationMode receiptValidationMode) {
		return DefaultBlockChainProcessor(blockHitPredicateFactory, batchEntityProcessor, receiptValidationMode);
	}
}}
