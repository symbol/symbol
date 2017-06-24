#pragma once
#include "catapult/model/ImportanceHeight.h"

namespace catapult { namespace state {

	/// Stateful catapult information.
	struct CatapultState {
	public:
		/// Creates a start state.
		CatapultState() : LastRecalculationHeight(0)
		{}

	public:
		/// The height at which importances were last recalculated.
		model::ImportanceHeight LastRecalculationHeight;
	};
}}
