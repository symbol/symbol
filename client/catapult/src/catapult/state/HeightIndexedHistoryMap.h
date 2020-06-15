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
#include "catapult/functions.h"
#include "catapult/types.h"
#include <map>

namespace catapult { namespace state {

	/// Map containing historical data indexed by height.
	template<typename TValue>
	class HeightIndexedHistoryMap {
	public:
		/// Gets the number of history entries.
		size_t size() const {
			return m_heightValueMap.size();
		}

		/// Gets the heights at which there is a value change.
		std::vector<Height> heights() const {
			auto heights = std::vector<Height>();
			for (auto iter = m_heightValueMap.crbegin(); m_heightValueMap.crend() != iter; ++iter)
				heights.push_back(iter->first);

			return heights;
		}

		/// Gets the current value.
		TValue get() const {
			return m_heightValueMap.empty() ? TValue() : (m_heightValueMap.cbegin())->second;
		}

		/// Gets the value at \a height.
		TValue get(Height height) const {
			auto iter = m_heightValueMap.lower_bound(height);
			return m_heightValueMap.cend() == iter ? TValue() : iter->second;
		}

		/// Returns \c true if \a predicate returns \c true for any historical value.
		bool anyOf(const predicate<const TValue&>& predicate) const {
			return std::any_of(m_heightValueMap.cbegin(), m_heightValueMap.cend(), [predicate](const auto& pair) {
				return predicate(pair.second);
			});
		}

	public:
		/// Adds \a value at \a height.
		void add(Height height, const TValue& value) {
			auto iter = m_heightValueMap.insert_or_assign(height, value).first;

			// delete larger height entry with same value
			if (m_heightValueMap.cbegin() != iter && (--iter)->second == value)
				m_heightValueMap.erase(iter);

			// delete new height entry if smaller height entry with same value exists
			if (get(height - Height(1)) == value)
				m_heightValueMap.erase(height);
		}

		/// Prunes all values less than \a height.
		/// \note Prune will never change the result of value queries at or after \a height.
		void prune(Height height) {
			auto iter = m_heightValueMap.lower_bound(height);
			if (m_heightValueMap.end() == iter)
				return;

			auto valueAtPruneHeight = iter->second;
			m_heightValueMap.erase(iter, m_heightValueMap.end());
			add(height, valueAtPruneHeight);
		}

	private:
		std::map<Height, TValue, std::greater<Height>> m_heightValueMap;
	};
}}
