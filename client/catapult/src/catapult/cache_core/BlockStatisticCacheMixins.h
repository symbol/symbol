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
#include "BlockStatisticCacheTypes.h"

namespace catapult { namespace cache {

	/// Mixin for retreiving a range of statistics.
	template<typename TSet>
	class BlockStatisticRangeMixin {
	public:
		/// Creates a mixin around \a set.
		explicit BlockStatisticRangeMixin(const TSet& set) : m_set(set)
		{}

	private:
		enum class FindIteratorScheme { Search, Lookup };
		using SearchFindIteratorFlag = std::integral_constant<FindIteratorScheme, FindIteratorScheme::Search>;
		using LookupFindIteratorFlag = std::integral_constant<FindIteratorScheme, FindIteratorScheme::Lookup>;

		template<typename T, typename = void>
		struct FindIteratorSchemeAccessor : SearchFindIteratorFlag {};

		template<typename T>
		struct FindIteratorSchemeAccessor<
				T,
				utils::traits::is_type_expression_t<decltype(reinterpret_cast<T*>(0)->findIterator(state::BlockStatistic()))>>
				: LookupFindIteratorFlag
		{};

	private:
		template<typename TIterableView>
		static auto FindIterator(SearchFindIteratorFlag, const TIterableView& view, const state::BlockStatistic& statistic) {
			// manually implement loop because vs has compile error with std::find
			for (auto iter = view.begin(); view.end() != iter; ++iter) {
				if (statistic == *iter)
					return iter;
			}

			return view.end();
		}

		template<typename TIterableView>
		static auto FindIterator(LookupFindIteratorFlag, const TIterableView& view, const state::BlockStatistic& statistic) {
			return view.findIterator(statistic);
		}

		template<typename TIterableView>
		static auto FindIterator(const TIterableView& view, const state::BlockStatistic& statistic) {
			return FindIterator(FindIteratorSchemeAccessor<TIterableView>(), view, statistic);
		}

	public:
		/// Gets a range object that spans \a count block statistics starting at the specified \a height.
		auto statistics(Height height, size_t count) const {
			if (m_set.empty()) {
				// note: this should not happen since the nemesis block is available from the beginning
				CATAPULT_THROW_RUNTIME_ERROR("block statistic cache is empty");
			}

			if (Height(0) == height || 0 == count)
				CATAPULT_THROW_INVALID_ARGUMENT("specified height or count out of range");

			auto iterableStatistics = MakeIterableView(m_set);
			auto lastIter = FindIterator(iterableStatistics, state::BlockStatistic(height));

			if (iterableStatistics.end() == lastIter)
				CATAPULT_THROW_INVALID_ARGUMENT_1("element with specified height not found", height);

			const auto& firstSetElement = *iterableStatistics.begin();
			const auto first = height.unwrap() - firstSetElement.Height.unwrap() < count - 1
					? state::BlockStatistic(firstSetElement.Height)
					: state::BlockStatistic(height - Height(count - 1));
			auto firstIter = FindIterator(iterableStatistics, first);

			return BlockStatisticRangeT<decltype(firstIter)>(firstIter, ++lastIter);
		}

	private:
		const TSet& m_set;
	};
}}
