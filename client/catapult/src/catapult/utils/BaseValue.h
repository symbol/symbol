#pragma once
#include "ImmutableValue.h"
#include <iosfwd>
#include <utility>

namespace catapult { namespace utils {

	/// Base class for immutable wrappers of basic types, to provide some type-safety.
	template<typename TValue, typename TTag, typename TBaseValue>
	struct BasicBaseValue : TTag {
	public:
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
			return m_value != rhs.m_value;
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

		/// Places \a baseValue into the stream \a out and returns the stream.
		friend std::ostream& operator<<(std::ostream& out, TBaseValue baseValue) {
			out << baseValue.m_value;
			return out;
		}

	private:
		ImmutableValue<TValue> m_value;
	};

	/// Immutable wrapper for basic types, to provide some type-safety.
	template<typename TValue, typename TTag>
	struct BaseValue : public BasicBaseValue<TValue, TTag, BaseValue<TValue, TTag>> {
	public:
		/// Creates a base value from a raw \a value.
		constexpr explicit BaseValue(TValue value = 0)
				: BasicBaseValue<TValue, TTag, BaseValue<TValue, TTag>>(value)
		{}

	public:
		/// Adds \a rhs and this value and returns a new value.
		constexpr BaseValue operator+(BaseValue rhs) const {
			return BaseValue(this->unwrap() + rhs.unwrap());
		}

		/// Subtracts \a rhs from this value and returns a new value.
		constexpr BaseValue operator-(BaseValue rhs) const {
			return BaseValue(this->unwrap() - rhs.unwrap());
		}
	};
}}
