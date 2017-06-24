#pragma once
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/types.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <functional>

namespace catapult { namespace model { struct Block; } }

namespace catapult { namespace chain {
	using BlockTarget = boost::multiprecision::uint256_t;

	/// Calculates the hit for a \a generationHash.
	uint64_t CalculateHit(const Hash256& generationHash);

	/// Calculates the score of \a currentBlock with parent \a parentBlock.
	uint64_t CalculateScore(const model::Block& parentBlock, const model::Block& currentBlock);

	/// Calculates the target from a time span (\a timeSpan), a \a difficulty and an effective signer importance
	/// of \a signerImportance for the block chain described by \a config.
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
		/// The generation hash.
		Hash256 GenerationHash;

		/// The time since the last block.
		utils::TimeSpan ElapsedTime;

		/// The public key of the block signer.
		Key Signer;

		/// The block difficulty.
		catapult::Difficulty Difficulty;

		/// The block height.
		catapult::Height Height;
	};

	/// Predicate used to determine if a block is a hit or not.
	class BlockHitPredicate {
	private:
		using ImportanceLookupFunc = std::function<Importance (const Key&, Height)>;

	public:
		/// Creates a predicate around a block chain configuration (\a config) and an importance lookup function
		/// (\a importanceLookup).
		BlockHitPredicate(
				const model::BlockChainConfiguration& config,
				const ImportanceLookupFunc& importanceLookup);

	public:
		/// Determines if the \a block is a hit given its parent (\a parentBlock) and generation hash (\a generationHash).
		bool operator()(const model::Block& parentBlock, const model::Block& block, const Hash256& generationHash) const;

		/// Determines if the specified \a context is a hit.
		bool operator()(const BlockHitContext& context) const;

	private:
		model::BlockChainConfiguration m_config;
		ImportanceLookupFunc m_importanceLookup;
	};
}}
