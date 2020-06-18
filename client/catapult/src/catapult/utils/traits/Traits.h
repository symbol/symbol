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

	// region is_scalar

	/// If T is a standard scalar type or a catapult scalar type, this struct will provide the member constant value equal to \c true.
	/// For any other type, value is \c false.
	template<typename T>
	struct is_scalar : std::is_scalar<T> {};

	template<typename X, typename Y>
	struct is_scalar<BaseValue<X, Y>> : std::true_type {};

	template<typename X, typename Y>
	struct is_scalar<ClampedBaseValue<X, Y>> : std::true_type {};

	template<typename X>
	struct is_scalar<ImmutableValue<X>> : std::true_type {};

	template<typename X>
	struct is_scalar<const X> : is_scalar<std::remove_const_t<X>> {};

	/// \c true if T is a standard scalar type or a catapult scalar type, \c false otherwise.
	template<typename T>
	inline constexpr bool is_scalar_v = is_scalar<T>::value;

	// endregion

	// region is_pod

	/// If T is a standard pod type or a catapult scalar type and is not a pointer type, this struct will provide the member constant
	/// value equal to \c true.
	/// For any other type, value is \c false.
	template<typename T>
	struct is_pod : std::integral_constant<bool, std::is_pod_v<T> && !std::is_pointer_v<T>> {};

	template<typename X, typename Y>
	struct is_pod<BaseValue<X, Y>> : std::true_type {};

	template<typename X, typename Y>
	struct is_pod<ClampedBaseValue<X, Y>> : std::true_type {};

	template<typename X>
	struct is_pod<ImmutableValue<X>> : std::true_type {};

	template<typename X>
	struct is_pod<const X> : is_pod<std::remove_const_t<X>> {};

	/// \c true if T is a standard pod type or a catapult scalar type and is not a pointer type, \c false otherwise.
	template<typename T>
	inline constexpr bool is_pod_v = is_pod<T>::value;

	// endregion

	// region is_base_of_ignore_reference

	/// Determines if \a X is a base of or same as \a Y (after stripping \a Y of references).
	template<typename X, typename Y>
	struct is_base_of_ignore_reference : std::is_base_of<X, std::remove_reference_t<Y>> {};

	/// \c true if \a X is a base of or same as \a Y (after stripping \a Y of references), \c false otherwise.
	template<typename X, typename Y>
	inline constexpr bool is_base_of_ignore_reference_v = is_base_of_ignore_reference<X, Y>::value;

	// endregion

	// region is_type_expression_t

	/// Type-based SFINAE helper that evaluates a type expression to a type (if valid) or void (if invalid).
	template<typename T, typename Enable = void>
	struct is_type_expression { using type = Enable; };

	/// \c true if the expression is valid and evaluates to a type, \c false otherwise.
	template<typename T, typename Enable = void>
	using is_type_expression_t = typename is_type_expression<T, Enable>::type;

	// endregion

	// region is_template_specialization

	/// If T is a specialization of TBase, this struct will provide the member constant value equal to \c true.
	/// For any other type, value is \c false.
	/// \note In order for detection to work, T must not have any const/volatile qualifiers.
	template<typename T, template<typename...> typename TBase>
	struct is_template_specialization : std::false_type {};

	template<template<typename...> typename TBase, typename... Args>
	struct is_template_specialization<TBase<Args...>, TBase> : std::true_type {};

	/// \c true if T is a specialization of TBase, \c false otherwise.
	template<typename T, template<typename...> typename TBase>
	inline constexpr bool is_template_specialization_v = is_template_specialization<T, TBase>::value;

	// endregion

	// region is_container

	/// If T is a container type, this struct will provide the member constant value equal to \c true.
	/// For any other type, value is \c false.
	template<typename T, typename = void>
	struct is_container : std::false_type {};

	template<typename T>
	struct is_container<T, is_type_expression_t<typename T::const_iterator>> : std::true_type {};

	/// \c true if T is a container type, \c false otherwise.
	template<typename T>
	inline constexpr bool is_container_v = is_container<T>::value;

	// endregion
}}}
