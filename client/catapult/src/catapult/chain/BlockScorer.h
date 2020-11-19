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
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/types.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <functional>

namespace catapult { namespace model { struct Block; } }

namespace catapult { namespace chain {

	using BlockTarget = boost::multiprecision::uint256_t;

	/// Calculates the hit for \a generationHash.
	uint64_t CalculateHit(const GenerationHash& generationHash);

	/// Calculates the score of \a currentBlock with parent \a parentBlock.
	uint64_t CalculateScore(const model::Block& parentBlock, const model::Block& currentBlock);

	/// Calculates the target from specified time span (\a timeSpan), \a difficulty and effective signer importance
	/// (\a signerImportance) for the block chain described by \a config.
	BlockTarget CalculateTarget(
			const utils::TimeSpan& timeSpan,
			Difficulty difficulty,
			Importance signerImportance,
			const model::BlockChainConfiguration& config);

	/// Calculates the target of \a currentBlock with parent \a parentBlock and effective signer importance
	/// of \a signerImportance for the block chain described by \a config.
	BlockTarget CalculateTarget(
			const model::Block& parentBlock,
			const model::Block& currentBlock,
			Importance signerImportance,
			const model::BlockChainConfiguration& config);

	/// Contextual information for calculating a block hit.
	struct BlockHitContext {
	public:
		/// Creates a block hit context.
		BlockHitContext() : ElapsedTime(utils::TimeSpan::FromSeconds(0))
		{}

	public:
		/// Generation hash.
		catapult::GenerationHash GenerationHash;

		/// Time since the last block.
		utils::TimeSpan ElapsedTime;

		/// Public key of the block signer.
		Key Signer;

		/// Block difficulty.
		catapult::Difficulty Difficulty;

		/// Block height.
		catapult::Height Height;
	};

	/// Predicate used to determine if a block is a hit or not.
	class BlockHitPredicate {
	private:
		using ImportanceLookupFunc = std::function<Importance (const Key&, Height)>;

	public:
		/// Creates a predicate around a block chain configuration (\a config) and an importance lookup function
		/// (\a importanceLookup).
		BlockHitPredicate(const model::BlockChainConfiguration& config, const ImportanceLookupFunc& importanceLookup);

	public:
		/// Determines if the \a block is a hit given its parent (\a parentBlock) and generation hash (\a generationHash).
		bool operator()(const model::Block& parentBlock, const model::Block& block, const GenerationHash& generationHash) const;

		/// Determines if the specified \a context is a hit.
		bool operator()(const BlockHitContext& context) const;

	private:
		model::BlockChainConfiguration m_config;
		ImportanceLookupFunc m_importanceLookup;
	};
}}
