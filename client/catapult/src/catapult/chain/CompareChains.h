#pragma once
#include "ChainComparisonCode.h"
#include "catapult/api/ChainApi.h"
#include "catapult/types.h"

namespace catapult { namespace chain {

	/// Options for comparing two chains.
	struct CompareChainsOptions {
		/// Creates compare-chains options.
		CompareChainsOptions() : CompareChainsOptions(0, 0)
		{}

		/// Creates compare-chains options from a maximum number of blocks to analyze (\a maxBlocksToAnalyze)
		/// and a maximum number of blocks to rewrite (\a maxBlocksToRewrite).
		CompareChainsOptions(
				uint32_t maxBlocksToAnalyze,
				uint32_t maxBlocksToRewrite)
				: MaxBlocksToAnalyze(maxBlocksToAnalyze)
				, MaxBlocksToRewrite(maxBlocksToRewrite)
		{}

		/// The maximum number of blocks to analyze.
		uint32_t MaxBlocksToAnalyze;

		/// The maximum number of blocks to rewrite.
		uint32_t MaxBlocksToRewrite;
	};

	/// The result of a chain comparison operation.
	struct CompareChainsResult {
		/// The end state of the chain comparison operation.
		ChainComparisonCode Code;

		/// The height of the last common block between the two chains.
		Height CommonBlockHeight;

		/// The depth of the fork that needs to be resolved.
		uint64_t ForkDepth;
	};

	/// Compares two chains (\a local and \a remote) with the specified \a options.
	thread::future<CompareChainsResult> CompareChains(
			const api::ChainApi& local,
			const api::ChainApi& remote,
			const CompareChainsOptions& options);
}}
