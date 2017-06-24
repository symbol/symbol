#pragma once
#include "ApiTypes.h"
#include "catapult/model/ChainScore.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/thread/Future.h"
#include "catapult/utils/NonCopyable.h"

namespace catapult { namespace api {

	/// Information about a chain.
	struct ChainInfo {
		/// The chain height.
		catapult::Height Height;

		/// The chain score.
		model::ChainScore Score;
	};

	/// An api for retrieving chain information from a node.
	class ChainApi : public utils::NonCopyable {
	public:
		virtual ~ChainApi() {}

	public:
		/// Gets information about the chain.
		virtual thread::future<ChainInfo> chainInfo() const = 0;

		/// Gets the hashes starting at \a height.
		virtual thread::future<model::HashRange> hashesFrom(Height height) const = 0;
	};
}}
