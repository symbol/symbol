#pragma once
#include "catapult/model/ChainScore.h"
#include "catapult/state/CatapultState.h"

namespace catapult { namespace cache {

	/// Chain supplemental data.
	struct SupplementalData {
		/// Catapult state.
		state::CatapultState State;

		/// Chain score.
		model::ChainScore ChainScore;
	};
}}
