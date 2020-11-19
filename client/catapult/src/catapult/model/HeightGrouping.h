/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

	/// Facade for working with grouped heights.
	template<typename TGroupedHeight>
	class HeightGroupingFacade {
	public:
		/// Creates a facade around \a groupedHeight and \a grouping and an optional \a message.
		HeightGroupingFacade(TGroupedHeight groupedHeight, Height::ValueType grouping, const char* message = nullptr)
				: m_groupedHeight(groupedHeight)
				, m_grouping(grouping) {
			if (0 == grouping)
				CATAPULT_THROW_INVALID_ARGUMENT_1("grouping must be non-zero", grouping);

			checkHeight(m_groupedHeight, message);
		}

	public:
		/// Calculates the grouped height \a count groupings backward.
		TGroupedHeight previous(size_t count) const {
			auto heightAdjustment = TGroupedHeight(m_grouping * count);
			return m_groupedHeight > heightAdjustment ? m_groupedHeight - heightAdjustment : TGroupedHeight(1);
		}

		/// Calculates the grouped height \a count groupings forward.
		TGroupedHeight next(size_t count) const {
			auto heightAdjustment = TGroupedHeight(m_grouping * count);
			auto nextHeight = m_groupedHeight + heightAdjustment;
			return TGroupedHeight(1) == m_groupedHeight && 1 != m_grouping && 0 != count
					? nextHeight - TGroupedHeight(1)
					: nextHeight;
		}

	protected:
		/// Checks that \a groupedHeight is valid given a descriptive \a message.
		void checkHeight(TGroupedHeight groupedHeight, const char* message) const {
			auto isHeightGroupingMultiple = 0 == groupedHeight.unwrap() % m_grouping;
			if (TGroupedHeight(1) == groupedHeight || (TGroupedHeight(0) != groupedHeight && isHeightGroupingMultiple))
				return;

			std::ostringstream out;
			if (message)
				out << message << " ";

			out << "grouped height " << groupedHeight << " is inconsistent with grouping " << m_grouping;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

	private:
		TGroupedHeight m_groupedHeight;
		Height::ValueType m_grouping;
	};

	struct ImportanceHeight_tag {};

	/// Represents a height at which importance is calculated.
	using ImportanceHeight = utils::BaseValue<Height::ValueType, ImportanceHeight_tag>;

	/// Calculates the importance height from the supplied \a height and \a grouping (the number of blocks that should be
	/// treated as a group for importance purposes).
	constexpr auto ConvertToImportanceHeight = CalculateGroupedHeight<ImportanceHeight>;
}}
