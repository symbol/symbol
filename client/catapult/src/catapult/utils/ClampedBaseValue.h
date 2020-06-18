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
#include "BaseValue.h"

namespace catapult { namespace utils {

	template<typename TValue, typename TRange>
	struct ClampedBaseValue;

	namespace detail {
		/// Struct that allows the range passed to ClampedBaseValue to be used as a tag without inheriting any
		/// of the range's traits.
		template<typename TRange>
		struct TaggedRange {};

		/// Defines types used in the definition of ClampedBaseValue.
		template<typename TValue, typename TRange>
		struct ClampedBaseValueTraits {
			using TaggedRangeType = TaggedRange<TRange>;

			using UnclampedType = BaseValue<TValue, TaggedRangeType>;

			using ClampedType = ClampedBaseValue<TValue, TRange>;

			using ClampedBaseType = BasicBaseValue<TValue, TaggedRangeType, ClampedType>;
		};
	}

	/// Base values that are constrained to a range of values.
	template<typename TValue, typename TRange>
	struct ClampedBaseValue : public detail::ClampedBaseValueTraits<TValue, TRange>::ClampedBaseType {
	private:
		using Traits = detail::ClampedBaseValueTraits<TValue, TRange>;

	public:
		/// Compatible unclamped type.
		using Unclamped = typename Traits::UnclampedType;

	public:
		/// Creates a clamped base value from a default value.
		constexpr ClampedBaseValue() : ClampedBaseValue(TRange::Default_Value)
		{}

		/// Creates a clamped base value from a raw \a value.
		constexpr explicit ClampedBaseValue(TValue value) : Traits::ClampedBaseType(Clamp(value))
		{}

	private:
		static constexpr TValue Clamp(TValue value) {
			return value < TRange::Min_Value
					? TRange::Min_Value
					: value > TRange::Max_Value ? TRange::Max_Value : value;
		}

	public:
		/// Gets the minimum clamped value.
		/// \note Function because ClampedBaseValue is incomplete (until closing brace).
		static constexpr ClampedBaseValue Min() {
			return ClampedBaseValue(TRange::Min_Value);
		}

		/// Gets the maximum clamped value.
		/// \note Function because ClampedBaseValue is incomplete (until closing brace).
		static constexpr ClampedBaseValue Max() {
			return ClampedBaseValue(TRange::Max_Value);
		}

	public:
		/// Adds \a rhs and this value and returns a new value
		constexpr ClampedBaseValue operator+(Unclamped rhs) const {
			return ClampedBaseValue(this->unwrap() + rhs.unwrap());
		}

		/// Subtracts \a rhs from this value and returns a new value.
		constexpr Unclamped operator-(ClampedBaseValue rhs) const {
			return Unclamped(this->unwrap() - rhs.unwrap());
		}
	};
}}
