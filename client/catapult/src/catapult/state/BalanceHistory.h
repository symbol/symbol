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
#include "catapult/types.h"
#include <map>

namespace catapult { namespace state {

	/// Represents the balance history of an account.
	class BalanceHistory {
	public:
		/// Gets the number of history entries.
		size_t size() const;

		/// Gets the heights at which there is a balance change.
		std::vector<Height> heights() const;

		/// Gets the current balance.
		Amount balance() const;

		/// Gets the balance at \a height.
		Amount balance(Height height) const;

		/// Returns \c true if any historical balance is at least \a amount.
		bool anyAtLeast(Amount amount) const;

	public:
		/// Adds \a balance at \a height.
		void add(Height height, Amount balance);

		/// Prunes all balances less than \a height.
		/// \note Prune will never change the result of balance queries at or after \a height.
		void prune(Height height);

	private:
		std::map<Height, Amount, std::greater<Height>> m_heightBalanceMap;
	};
}}
