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
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/AccountStateCacheUtils.h"
#include "catapult/chain/ChainResults.h"
#include "catapult/chain/ChainUtils.h"
#include "catapult/io/BlockStatementSerializer.h"
#include "catapult/io/Stream.h"
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

		// region log formatters

		class BufferedOutputStream : public io::OutputStream {
		public:
			explicit BufferedOutputStream(std::vector<uint8_t>& buffer) : m_buffer(buffer)
			{}

		public:
			void write(const RawBuffer& buffer) override {
				m_buffer.insert(m_buffer.end(), buffer.pData, buffer.pData + buffer.Size);
			}

			void flush() override
			{}

		private:
			std::vector<uint8_t>& m_buffer;
		};

		std::string FormatCacheStateLog(Height height, const cache::StateHashInfo& stateHashInfo) {
			std::ostringstream out;
			out
					<< "cache state hash (" << stateHashInfo.SubCacheMerkleRoots.size() << " components) at height " << height
					<< std::endl << stateHashInfo.StateHash;

			for (const auto& subCacheMerkleRoot : stateHashInfo.SubCacheMerkleRoots)
				out << std::endl << " + " << subCacheMerkleRoot;

			auto log = out.str();
			CATAPULT_LOG(trace) << log;
			return log;
		}

		std::string FormatBlockStatementLog(const model::BlockStatement& blockStatement) {
			std::ostringstream out;

			std::vector<uint8_t> buffer;
			BufferedOutputStream outputStream(buffer);
			io::WriteBlockStatement(blockStatement, outputStream);

			out << utils::HexFormat(buffer);
			return out.str();
		}

		// endregion

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
				std::vector<std::string> cacheStateLogs;
				cacheStateLogs.push_back(FormatCacheStateLog(pParent->Height, state.Cache.calculateStateHash(pParent->Height)));

				for (auto& element : elements) {
					// 1. check generation hash
					auto result = CheckGenerationHash(element, *pParent, *pParentGenerationHash, blockHitPredicate, readOnlyCache);
					if (!IsValidationResultSuccess(result))
						return result;

					// 2. validate and observe block
					model::BlockStatementBuilder blockStatementBuilder;
					auto blockDependentState = createBlockDependentObserverState(state, blockStatementBuilder);

					const auto& block = element.Block;
					result = m_batchEntityProcessor(block.Height, block.Timestamp, ExtractEntityInfos(element), blockDependentState);
					if (!IsValidationResultSuccess(result)) {
						CATAPULT_LOG(warning) << "batch processing of block " << block.Height << " failed with " << result;
						return result;
					}

					// 3. check state hash
					if (!CheckStateHash(element, state.Cache, cacheStateLogs))
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
						: observers::ObserverState(state.Cache, blockStatementBuilder);
			}

		private:
			static Key GetVrfPublicKey(const cache::ReadOnlyAccountStateCache& accountStateCache, const Address& blockHarvester) {
				Key vrfPublicKey;
				cache::ProcessForwardedAccountState(accountStateCache, blockHarvester, [&vrfPublicKey](const auto& accountState) {
					vrfPublicKey = state::GetVrfPublicKey(accountState);
				});
				return vrfPublicKey;
			}

			static validators::ValidationResult CheckGenerationHash(
					model::BlockElement& element,
					const model::Block& parentBlock,
					const GenerationHash& parentGenerationHash,
					const BlockHitPredicate& blockHitPredicate,
					const cache::ReadOnlyCatapultCache& readOnlyCache) {
				const auto& block = element.Block;
				const auto& accountStateCache = readOnlyCache.sub<cache::AccountStateCache>();

				auto accountStateIter = accountStateCache.find(block.SignerPublicKey);
				if (!accountStateIter.tryGet()) {
					CATAPULT_LOG(warning)
							<< "block signer at height " << block.Height
							<< " is not present in account state cache " << block.SignerPublicKey;
					return chain::Failure_Chain_Block_Unknown_Signer;
				}

				auto vrfPublicKey = GetVrfPublicKey(accountStateCache, accountStateIter.get().Address);
				auto vrfVerifyResult = crypto::VerifyVrfProof(block.GenerationHashProof, parentGenerationHash, vrfPublicKey);

				if (Hash512() == vrfVerifyResult) {
					CATAPULT_LOG(warning) << "vrf proof does not validate at height " << block.Height;
					return chain::Failure_Chain_Block_Invalid_Vrf_Proof;
				}

				element.GenerationHash = vrfVerifyResult.copyTo<GenerationHash>();
				if (!blockHitPredicate(parentBlock, block, element.GenerationHash)) {
					CATAPULT_LOG(warning) << "block " << block.Height << " failed hit";
					return chain::Failure_Chain_Block_Not_Hit;
				}

				return validators::ValidationResult::Success;
			}

			static bool CheckStateHash(
					model::BlockElement& element,
					cache::CatapultCacheDelta& cacheDelta,
					std::vector<std::string>& cacheStateLogs) {
				const auto& block = element.Block;
				auto cacheStateHashInfo = cacheDelta.calculateStateHash(block.Height);
				cacheStateLogs.push_back(FormatCacheStateLog(block.Height, cacheStateHashInfo));

				if (block.StateHash != cacheStateHashInfo.StateHash) {
					CATAPULT_LOG(warning)
							<< "block state hash (" << block.StateHash << ") does not match "
							<< "cache state hash (" << cacheStateHashInfo.StateHash << ") "
							<< "at height " << block.Height;

					for (const auto& log : cacheStateLogs)
						CATAPULT_LOG(info) << log;

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
				auto calculatedReceiptsHash = Hash256();
				if (ReceiptValidationMode::Enabled == receiptValidationMode) {
					auto pBlockStatement = blockStatementBuilder.build();
					calculatedReceiptsHash = CalculateMerkleHash(*pBlockStatement);
					element.OptionalStatement = std::move(pBlockStatement);
				}

				if (block.ReceiptsHash != calculatedReceiptsHash) {
					CATAPULT_LOG(warning)
							<< "block receipts hash (" << block.ReceiptsHash << ") does not match "
							<< "calculated receipts hash (" << calculatedReceiptsHash << ") "
							<< "at height " << block.Height;

					if (element.OptionalStatement)
						CATAPULT_LOG(info) << FormatBlockStatementLog(*element.OptionalStatement);

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
