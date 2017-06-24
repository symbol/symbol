#pragma once
#include "catapult/types.h"
#include <functional>
#include <memory>
#include <unordered_set>

namespace catapult { namespace model { class ChainScore; } }

namespace catapult { namespace local { namespace api {

	/// Interface for accessing api chain score.
	class ChainScoreProvider {
	public:
		virtual ~ChainScoreProvider()
		{}

	public:
		/// Save score (\a chainScore).
		virtual void saveScore(const model::ChainScore& chainScore) = 0;

		/// Load score. If no score has been saved 0 score will be returned.
		virtual model::ChainScore loadScore() const = 0;
	};
}}}
