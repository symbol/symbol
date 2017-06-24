#pragma once
#include "catapult/types.h"

namespace catapult { namespace model {

	struct ImportanceHeight_tag {};

	/// Represents a height at which importance is calculated.
	using ImportanceHeight = utils::BaseValue<Height::ValueType, ImportanceHeight_tag>;

	/// Calculates the importance height from the supplied \a height and \a grouping (the number of blocks that should be
	/// treated as a group for importance purposes).
	ImportanceHeight ConvertToImportanceHeight(Height height, Height::ValueType grouping);
}}
