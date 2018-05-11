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
#include "ImmutableValue.h"
#include "catapult/types.h"

namespace catapult { namespace utils {

	struct XemAmount_tag {};

	/// A non-fractional amount of xem.
	using XemAmount = utils::BaseValue<uint64_t, XemAmount_tag>;

	/// Represents units of xem.
	class XemUnit {
	public:
		/// Raw value type.
		using ValueType = uint64_t;

	private:
		static constexpr uint64_t Microxem_Per_Xem = 1'000'000;

	public:
		/// Creates a zeroed xem unit.
		constexpr XemUnit() : m_value(0)
		{}

		/// Creates a xem unit from \a amount microxem.
		constexpr explicit XemUnit(Amount amount) : m_value(amount.unwrap())
		{}

		/// Creates a xem unit from \a amount xem.
		constexpr explicit XemUnit(XemAmount amount) : m_value(amount.unwrap() * Microxem_Per_Xem)
		{}

		/// Creates a copy of \a rhs.
		constexpr XemUnit(const XemUnit& rhs) : m_value(static_cast<ValueType>(rhs.m_value))
		{}

	public:
		/// Assigns \a rhs to this.
		XemUnit& operator=(XemUnit rhs) {
			m_value = std::move(rhs.m_value);
			return *this;
		}

		/// Assigns \a rhs to this.
		XemUnit& operator=(Amount rhs) {
			m_value = XemUnit(rhs).m_value;
			return *this;
		}

		/// Assigns \a rhs to this.
		XemUnit& operator=(XemAmount rhs) {
			m_value = XemUnit(rhs).m_value;
			return *this;
		}

	public:
		/// Gets the number of (whole-unit) xem in this unit.
		constexpr XemAmount xem() const {
			return XemAmount(m_value / Microxem_Per_Xem);
		}

		/// Gets the number of micronem in this unit.
		constexpr Amount microxem() const {
			return Amount(m_value);
		}

		/// Returns \c true if this unit includes fractional xem.
		constexpr bool isFractional() const {
			return 0 != m_value % Microxem_Per_Xem;
		}

	public:
		/// Returns \c true if this value is equal to \a rhs.
		constexpr bool operator==(XemUnit rhs) const {
			return m_value == rhs.m_value;
		}

		/// Returns \c true if this value is not equal to \a rhs.
		constexpr bool operator!=(XemUnit rhs) const {
			return !(*this == rhs);
		}

		/// Insertion operator for outputting \a unit to \a out.
		friend std::ostream& operator<<(std::ostream& out, XemUnit unit);

	private:
		utils::ImmutableValue<ValueType> m_value;
	};

	/// Tries to parse \a str into a XemUnit (\a parsedValue).
	/// \note Only non-fractional xem units are parsed successfully.
	bool TryParseValue(const std::string& str, XemUnit& parsedValue);
}}
