#pragma once
#include "catapult/cache/ShortHashPair.h"
#include "catapult/model/CosignedTransactionInfo.h"
#include "catapult/functions.h"
#include <vector>

namespace catapult { namespace partialtransaction {

	/// A vector of cosigned (partial) transaction infos.
	using CosignedTransactionInfos = std::vector<model::CosignedTransactionInfo>;

	/// Prototype for a function that retrieves partial transaction infos given a set of short hash pairs.
	using CosignedTransactionInfosRetriever = std::function<CosignedTransactionInfos (const cache::ShortHashPairMap&)>;

	/// Function signature for consuming a vector of cosigned transaction infos.
	using CosignedTransactionInfosConsumer = consumer<CosignedTransactionInfos&&>;

	/// Function signature for supplying a range of short hash pairs.
	using ShortHashPairsSupplier = supplier<cache::ShortHashPairRange>;
}}
