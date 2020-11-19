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

#pragma once
#include "catapult/chain/BatchEntityProcessor.h"
#include "catapult/disruptor/DisruptorElement.h"
#include "catapult/model/WeakEntityInfo.h"
#include <functional>

namespace catapult {
	namespace cache { class ReadOnlyCatapultCache; }
	namespace chain { struct ObserverState; }
}

namespace catapult { namespace consumers {

	/// Tuple composed of a block, a hash and a generation hash.
	class WeakBlockInfo : public model::WeakEntityInfoT<model::Block> {
	public:
			/// Creates a block info.
			constexpr WeakBlockInfo() : m_pGenerationHash(nullptr)
			{}

			/// Creates a block info around \a blockElement.
			constexpr explicit WeakBlockInfo(const model::BlockElement& blockElement)
					: WeakEntityInfoT(blockElement.Block, blockElement.EntityHash)
					, m_pGenerationHash(&blockElement.GenerationHash)
			{}

		public:
			/// Gets the generation hash.
			constexpr const GenerationHash& generationHash() const {
				return *m_pGenerationHash;
			}

		private:
			const GenerationHash* m_pGenerationHash;
	};

	/// Function signature for validating, executing and updating a partial block chain given a parent block info
	/// and updating a cache.
	using BlockChainProcessor = std::function<validators::ValidationResult (
			const WeakBlockInfo&,
			disruptor::BlockElements&,
			observers::ObserverState&)>;

	/// Predicate for determining whether or not two blocks form a hit.
	using BlockHitPredicate = predicate<const model::Block&, const model::Block&, const GenerationHash&>;

	/// Factory for creating a predicate for determining whether or not two blocks form a hit.
	using BlockHitPredicateFactory = std::function<BlockHitPredicate (const cache::ReadOnlyCatapultCache&)>;

	/// Possible receipt validation modes.
	enum class ReceiptValidationMode {
		/// Disabled, skip validation of receipts.
		Disabled,

		/// Enabled, generate and validate receipts.
		Enabled
	};

	/// Creates a block chain processor around the specified block hit predicate factory (\a blockHitPredicateFactory)
	/// and batch entity processor (\a batchEntityProcessor) with \a receiptValidationMode.
	BlockChainProcessor CreateBlockChainProcessor(
			const BlockHitPredicateFactory& blockHitPredicateFactory,
			const chain::BatchEntityProcessor& batchEntityProcessor,
			ReceiptValidationMode receiptValidationMode);
}}
