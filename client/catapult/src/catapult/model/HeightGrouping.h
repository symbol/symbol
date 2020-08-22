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
#include "catapult/exceptions.h"
#include "catapult/types.h"

namespace catapult { namespace model {

	/// Calculates the grouped height from the supplied \a height and \a grouping (the number of blocks that should be treated as a group
	/// for calculation purposes).
	template<typename TGroupedHeight>
	TGroupedHeight CalculateGroupedHeight(Height height, Height::ValueType grouping) {
		if (0 == grouping)
			CATAPULT_THROW_INVALID_ARGUMENT_1("grouping must be non-zero", grouping);

		Height::ValueType previousHeight = height.unwrap() - 1;
		Height::ValueType groupedHeight = (previousHeight / grouping) * grouping;
		return TGroupedHeight(groupedHeight < 1 ? 1 : groupedHeight);
	}

	struct ImportanceHeight_tag {};

	/// Represents a height at which importance is calculated.
	using ImportanceHeight = utils::BaseValue<Height::ValueType, ImportanceHeight_tag>;

	/// Calculates the importance height from the supplied \a height and \a grouping (the number of blocks that should be
	/// treated as a group for importance purposes).
	constexpr auto ConvertToImportanceHeight = CalculateGroupedHeight<ImportanceHeight>;
}}
