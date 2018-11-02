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
#include "catapult/utils/ClampedBaseValue.h"
#include <type_traits>

namespace catapult { namespace utils { namespace traits {

	/// If T is a standard scalar type or a catapult scalar type, this struct will provide the member constant value equal to \c true.
	/// For any other type, value is \c false.
	template<typename T>
	struct is_scalar : std::is_scalar<T>
	{};

	template<typename X, typename Y>
	struct is_scalar<BaseValue<X, Y>> : std::true_type
	{};

	template<typename X, typename Y>
	struct is_scalar<ClampedBaseValue<X, Y>> : std::true_type
	{};

	template<typename X>
	struct is_scalar<ImmutableValue<X>> : std::true_type
	{};

	template<typename X>
	struct is_scalar<const X> : is_scalar<typename std::remove_const<X>::type>
	{};

	/// If T is a standard pod type or a catapult scalar type and is not a pointer type, this struct will provide the member constant
	/// value equal to \c true.
	/// For any other type, value is \c false.
	template<typename T>
	struct is_pod : std::integral_constant<bool, std::is_pod<T>::value && !std::is_pointer<T>::value>
	{};

	template<typename X, typename Y>
	struct is_pod<BaseValue<X, Y>> : std::true_type
	{};

	template<typename X, typename Y>
	struct is_pod<ClampedBaseValue<X, Y>> : std::true_type
	{};

	template<typename X>
	struct is_pod<ImmutableValue<X>> : std::true_type
	{};

	template<typename X>
	struct is_pod<const X> : is_pod<typename std::remove_const<X>::type>
	{};

	/// Determines if \a X is a base of or same as \a Y (after stripping \a Y of references).
	template<typename X, typename Y>
	struct is_base_of_ignore_reference : std::is_base_of<
			X,
			typename std::remove_reference<Y>::type>
	{};

	/// Hides a template specialization if \a X is a base of or same as \a Y (after stripping \a Y of references).
	/// \note This can be used to allow copy construction when a perfectly forwarding constructor is used.
	template<typename X, typename Y>
	using disable_if_same_or_derived = typename std::enable_if<!is_base_of_ignore_reference<X, Y>::value>::type;

	/// Type-based SFINAE helper that exposes a `type` alias that evaluates to either (1) the desired subtype alias if present
	/// or (2) void if the desired subtype alias is not present.
	template<typename T, typename Enable = void>
	struct enable_if_type { using type = Enable; };
}}}
