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
#include <type_traits>

namespace catapult { namespace test {

	/// Type convertibility tests.
	class TypeConvertibilityTests {
	private:
		template<typename... T>
		struct TypeSet {};

	public:
		/// Asserts specified types are not convertible.
		template<typename... T>
		static void AssertCannotConvertTypes() {
			DispatchOuter(TypeSet<T...>(), TypeSet<T...>());
		}

	private:
		template<typename T, typename U>
		static void Create(T, U) {
			auto isConvertible = std::is_convertible_v<T, U>;
			if (std::is_same_v<T, U>)
				EXPECT_TRUE(isConvertible);
			else
				EXPECT_FALSE(isConvertible);
		}

		template<typename ElemOuter, typename Head, typename... Tail>
		static void DispatchInner(ElemOuter, TypeSet<Head, Tail...>) {
			Create(ElemOuter(), Head());
			DispatchInner(ElemOuter(), TypeSet<Tail...>());
		}

		template<typename ElemOuter, typename... Tail>
		static void DispatchInner(ElemOuter, TypeSet<>) {
			// finished iteration of inner loop
		}

		template<typename TSetInner, typename Head, typename... Tail>
		static void DispatchOuter(TypeSet<Head, Tail...>, TSetInner) {
			DispatchInner(Head(), TSetInner());
			DispatchOuter(TypeSet<Tail...>(), TSetInner());
		}

		template<typename TSetInner, typename... Tail>
		static void DispatchOuter(TypeSet<>, TSetInner) {
			// finished iteration of outer loop
		}
	};
}}
