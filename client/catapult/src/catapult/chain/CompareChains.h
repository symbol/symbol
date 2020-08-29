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

#pragma once
#include "ChainComparisonCode.h"
#include "catapult/api/ChainApi.h"
#include "catapult/types.h"

namespace catapult { namespace chain {

	/// Options for comparing two chains.
	struct CompareChainsOptions {
		/// Number of hashes to pull per batch.
		uint32_t HashesPerBatch;

		/// Finalized height supplier.
		supplier<Height> FinalizedHeightSupplier;
	};

	/// Result of a chain comparison operation.
	struct CompareChainsResult {
		/// End state of the chain comparison operation.
		ChainComparisonCode Code;

		/// Height of the last common block between the two chains.
		Height CommonBlockHeight;

		/// Depth of the fork that needs to be resolved.
		uint64_t ForkDepth;
	};

	/// Compares two chains (\a local and \a remote) with the specified \a options.
	thread::future<CompareChainsResult> CompareChains(
			const api::ChainApi& local,
			const api::ChainApi& remote,
			const CompareChainsOptions& options);
}}
