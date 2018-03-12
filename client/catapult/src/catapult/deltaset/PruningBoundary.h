#pragma once
#include <memory>

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

		/// Returns the pruning boundary value.
		constexpr const T& value() const {
			return m_value;
		}

	private:
		bool m_isSet;
		T m_value;
	};

	template<typename T>
	class PruningBoundary<std::shared_ptr<T>> {
	public:
		constexpr PruningBoundary() : m_isSet(false)
		{}

		constexpr PruningBoundary(const std::shared_ptr<T>& pValue) : m_isSet(true), m_pValue(pValue)
		{}

	public:
		constexpr bool isSet() const {
			return m_isSet;
		}

		constexpr std::shared_ptr<T> value() const {
			return m_pValue;
		}

	private:
		bool m_isSet;
		std::shared_ptr<T> m_pValue;
	};
}}
