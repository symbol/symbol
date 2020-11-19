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

namespace catapult { namespace deltaset {

	/// Represents an optional pruning boundary.
	template<typename T>
	class PruningBoundary {
	public:
		/// Creates a null pruning boundary.
		constexpr PruningBoundary() : m_isSet(false), m_value()
		{}

		/// Creates a pruning boundary around \a value.
		constexpr PruningBoundary(const T& value) : m_isSet(true), m_value(value)
		{}

	public:
		/// Returns \c true if the pruning boundary value is set.
		constexpr bool isSet() const {
			return m_isSet;
		}

		/// Gets the pruning boundary value.
		constexpr const T& value() const {
			return m_value;
		}

	private:
		bool m_isSet;
		T m_value;
	};
}}
