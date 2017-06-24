#pragma once
#include "NonCopyable.h"

namespace catapult { namespace utils {

#pragma pack(push, 1)

	/// A move-only value wrapper.
	template<typename TValue>
	class ImmutableValue : public utils::MoveOnly {
	public:
		using TImmutableValue = ImmutableValue<TValue>;

	public:
		/// Creates an immutable value around \a value.
		constexpr explicit ImmutableValue(TValue value) : m_value(value)
		{}

	public:
		/// Casts this immutable value to a mutable value.
		constexpr operator const TValue() const {
			return m_value;
		}

	private:
		TValue m_value;
	};

#pragma pack(pop)
}}
