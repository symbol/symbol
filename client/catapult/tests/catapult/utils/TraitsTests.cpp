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

#include "catapult/utils/traits/Traits.h"
#include "catapult/types.h"
#include "tests/TestHarness.h"
#include <array>

namespace catapult { namespace utils {

#define TEST_CLASS TraitsTests

	// region is_scalar / is_pod

	namespace {
		template<typename T>
		void AssertIsScalar(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_TRUE(traits::is_scalar<T>::value) << lineNumber;
			EXPECT_TRUE(traits::is_scalar<const T>::value) << lineNumber;
		}

		template<typename T>
		void AssertIsNotScalar(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_FALSE(traits::is_scalar<T>::value) << lineNumber;
			EXPECT_FALSE(traits::is_scalar<const T>::value) << lineNumber;
		}

		template<typename T>
		void AssertIsPod(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_TRUE(traits::is_pod<T>::value) << lineNumber;
			EXPECT_TRUE(traits::is_pod<const T>::value) << lineNumber;
		}

		template<typename T>
		void AssertIsNotPod(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_FALSE(traits::is_pod<T>::value) << lineNumber;
			EXPECT_FALSE(traits::is_pod<const T>::value) << lineNumber;
		}
	}

	TEST(TEST_CLASS, IsScalarReturnsTrueOnlyForIntegralAndIntegralLikeTypes) {
		// Assert:
		// - basic and pointer types
		AssertIsScalar<int>(__LINE__);
		AssertIsScalar<void*>(__LINE__);
		AssertIsNotScalar<std::shared_ptr<int>>(__LINE__);
		AssertIsNotScalar<std::unique_ptr<int>>(__LINE__);

		// - catapult scalars
		AssertIsScalar<Height>(__LINE__); // well known base value
		AssertIsScalar<Difficulty>(__LINE__); // well known clamped base value
		AssertIsScalar<BaseValue<short, void>>(__LINE__); // arbitrary base value
		AssertIsScalar<ClampedBaseValue<int32_t, void>>(__LINE__); // arbitrary clamped base value
		AssertIsScalar<ImmutableValue<uint64_t>>(__LINE__); // arbitrary immutable value

		// - compound types
		AssertIsNotScalar<std::vector<int>>(__LINE__);
		AssertIsNotScalar<std::array<unsigned char, 1>>(__LINE__);
	}

	TEST(TEST_CLASS, IsPodReturnsTrueOnlyForIntegralAndIntegralLikeTypes) {
		// Assert:
		// - basic and pointer types
		AssertIsPod<int>(__LINE__);
		AssertIsNotPod<void*>(__LINE__);
		AssertIsNotPod<std::shared_ptr<int>>(__LINE__);
		AssertIsNotPod<std::unique_ptr<int>>(__LINE__);

		// - catapult scalars
		AssertIsPod<Height>(__LINE__); // well known base value
		AssertIsPod<Difficulty>(__LINE__); // well known clamped base value
		AssertIsPod<BaseValue<short, void>>(__LINE__); // arbitrary base value
		AssertIsPod<ClampedBaseValue<int32_t, void>>(__LINE__); // arbitrary clamped base value
		AssertIsPod<ImmutableValue<uint64_t>>(__LINE__); // arbitrary immutable value

		// - compound types
		AssertIsNotPod<std::vector<int>>(__LINE__);
		AssertIsPod<std::array<unsigned char, 1>>(__LINE__);
	}

	// endregion

	// region is_base_of_ignore_reference

	namespace {
		struct Base {};

		struct Derived : Base {};

#define EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(EXPECTED, TLEFT, TRIGHT) \
	EXPECT_EQ(EXPECTED, (traits::is_base_of_ignore_reference<TLEFT, TRIGHT>::value)); \
	EXPECT_EQ(EXPECTED, (traits::is_base_of_ignore_reference<TLEFT, TRIGHT&>::value)); \
	EXPECT_EQ(EXPECTED, (traits::is_base_of_ignore_reference<TLEFT, TRIGHT&&>::value));
	}

	TEST(TEST_CLASS, IsBaseOfIgnoreReferenceReturnsTrueIfLeftStrippedOfReferenceIsDerivedFromBase) {
		// Assert:
		// - (Base, Base) with any reference qualifiers is true
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(true, Base, Base);

		// - (Base, Derived) with any reference qualifiers is true
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(true, Base, Derived);

		// - (Derived, Base) with any reference qualifiers is false
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(false, Derived, Base);

		// - (Base, int) with any reference qualifiers is false
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(false, Base, int);

		// - (Base, Base) with any cv qualifiers is true
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(true, Base, const Base);
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(true, Base, const volatile Base);
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(true, Base, volatile Base);

		// - (Base, Derived) with any cv qualifiers is true
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(true, Base, const Derived);
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(true, Base, const volatile Derived);
		EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(true, Base, volatile Derived);

		// - references are not stripped from the left
		EXPECT_FALSE((traits::is_base_of_ignore_reference<Base&, Base>::value));
		EXPECT_FALSE((traits::is_base_of_ignore_reference<Base&&, Base>::value));
	}

	// endregion

	// region enable_if_type

	namespace {
		template<typename T, typename = void>
		struct FooTagger {
			static constexpr auto Tag = 1u;
		};

		template<typename T>
		struct FooTagger<T, typename traits::enable_if_type<typename T::Foo>::type> {
			static constexpr auto Tag = 2u;
		};
	}

	TEST(TEST_CLASS, EnableIfTypeCanBeUsedToConditionallySelectTypeBasedOnPresenceOfSubTypeAlias) {
		// Arrange:
		struct FooContainer { using Foo = int; };
		struct OtherContainer {};

		// Assert: FooTagger::Tag is 2u iff T contains a `Foo` type alias
		EXPECT_EQ(2u, static_cast<uint32_t>(FooTagger<FooContainer>::Tag));
		EXPECT_EQ(1u, static_cast<uint32_t>(FooTagger<OtherContainer>::Tag));
	}

	// endregion
}}
