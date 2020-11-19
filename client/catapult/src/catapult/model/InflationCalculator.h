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
#include "catapult/types.h"
#include <map>

namespace catapult { namespace model {

	/// Calculator for calculating the inflation at a given height and the total inflation up to a given height.
	class InflationCalculator {
	public:
		/// Gets the number of inflation entries.
		size_t size() const;

		/// Returns \c true if the inflation map contains entry with \a height and \a amount.
		bool contains(Height height, Amount amount) const;

		/// Gets the inflation amount at \a height.
		Amount getSpotAmount(Height height) const;

		/// Gets the total inflation amount up to (but not including) \a height.
		Amount getCumulativeAmount(Height height) const;

		/// Calculates the total inflation.
		std::pair<Amount, bool> sumAll() const;

	public:
		/// Adds inflation of \a amount starting at \a height.
		void add(Height height, Amount amount);

	private:
		std::map<Height, Amount> m_inflationMap;
	};
}}
