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
#include <iosfwd>
#include <utility>

namespace catapult { namespace utils {

	/// Base class for immutable wrappers of basic types, to provide some type-safety.
	template<typename TValue, typename TTag, typename TBaseValue>
	class BasicBaseValue : TTag {
	public:
		/// Raw value type.
		using ValueType = TValue;

	public:
		/// Creates a basic base value from a raw \a value.
		constexpr explicit BasicBaseValue(TValue value) : m_value(value)
		{}

		/// Creates a copy of \a rhs.
		constexpr BasicBaseValue(const BasicBaseValue& rhs) : m_value(static_cast<TValue>(rhs.m_value))
		{}

	public:
		/// Assigns \a rhs to this.
		BasicBaseValue& operator=(BasicBaseValue rhs) {
			m_value = std::move(rhs.m_value);
			return *this;
		}

	public:
		/// Unwraps this value and returns the underlying raw value.
		constexpr ValueType unwrap() const {
			return m_value;
		}

	public:
		/// Returns \c true if this value is equal to \a rhs.
		constexpr bool operator==(TBaseValue rhs) const {
			return m_value == rhs.m_value;
		}

		/// Returns \c true if this value is not equal to \a rhs.
		constexpr bool operator!=(TBaseValue rhs) const {
			return !(*this == rhs);
		}

		/// Returns \c true if this value is greater than or equal to \a rhs.
		constexpr bool operator>=(TBaseValue rhs) const {
			return m_value >= rhs.m_value;
		}

		/// Returns \c true if this value is greater than \a rhs.
		constexpr bool operator>(TBaseValue rhs) const {
			return m_value > rhs.m_value;
		}

		/// Returns \c true if this value is less than or equal to \a rhs.
		constexpr bool operator<=(TBaseValue rhs) const {
			return m_value <= rhs.m_value;
		}

		/// Returns \c true if this value is less than \a rhs.
		constexpr bool operator<(TBaseValue rhs) const {
			return m_value < rhs.m_value;
		}

		/// Insertion operator for outputting \a baseValue to \a out.
		friend std::ostream& operator<<(std::ostream& out, TBaseValue baseValue) {
			out << baseValue.m_value;
			return out;
		}

	private:
		ImmutableValue<TValue> m_value;
	};

	/// Immutable wrapper for basic types, to provide some type-safety.
	template<typename TValue, typename TTag>
	class BaseValue : public BasicBaseValue<TValue, TTag, BaseValue<TValue, TTag>> {
	public:
		/// Creates a base value from a raw \a value.
		constexpr explicit BaseValue(TValue value = 0)
				: BasicBaseValue<TValue, TTag, BaseValue<TValue, TTag>>(value)
		{}

	public:
		/// Adds \a rhs and this value and returns a new value.
		constexpr BaseValue operator+(BaseValue rhs) const {
			return BaseValue(static_cast<TValue>(this->unwrap() + rhs.unwrap()));
		}

		/// Subtracts \a rhs from this value and returns a new value.
		constexpr BaseValue operator-(BaseValue rhs) const {
			return BaseValue(static_cast<TValue>(this->unwrap() - rhs.unwrap()));
		}
	};
}}
