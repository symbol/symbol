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

			EXPECT_TRUE(traits::is_scalar_v<T>) << lineNumber;
			EXPECT_TRUE(traits::is_scalar_v<const T>) << lineNumber;
		}

		template<typename T>
		void AssertIsNotScalar(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_FALSE(traits::is_scalar<T>::value) << lineNumber;
			EXPECT_FALSE(traits::is_scalar<const T>::value) << lineNumber;

			EXPECT_FALSE(traits::is_scalar_v<T>) << lineNumber;
			EXPECT_FALSE(traits::is_scalar_v<const T>) << lineNumber;
		}

		template<typename T>
		void AssertIsPod(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_TRUE(traits::is_pod<T>::value) << lineNumber;
			EXPECT_TRUE(traits::is_pod<const T>::value) << lineNumber;

			EXPECT_TRUE(traits::is_pod_v<T>) << lineNumber;
			EXPECT_TRUE(traits::is_pod_v<const T>) << lineNumber;
		}

		template<typename T>
		void AssertIsNotPod(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_FALSE(traits::is_pod<T>::value) << lineNumber;
			EXPECT_FALSE(traits::is_pod<const T>::value) << lineNumber;

			EXPECT_FALSE(traits::is_pod_v<T>) << lineNumber;
			EXPECT_FALSE(traits::is_pod_v<const T>) << lineNumber;
		}
	}

	TEST(TEST_CLASS, IsScalarReturnsTrueOnlyForIntegralAndIntegralLikeTypes) {
		// Assert: basic and pointer types
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
		AssertIsNotScalar<std::array<uint8_t, 1>>(__LINE__);
	}

	TEST(TEST_CLASS, IsPodReturnsTrueOnlyForIntegralAndIntegralLikeTypes) {
		// Assert: basic and pointer types
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
		AssertIsPod<std::array<uint8_t, 1>>(__LINE__);
	}

	// endregion

	// region is_base_of_ignore_reference

	namespace {
		struct Base {};

		struct Derived : Base {};
	}

#define EXPECT_IS_BASE_OF_IGNORE_REFERENCE_RESULT(EXPECTED, TLEFT, TRIGHT) \
	do { \
		EXPECT_EQ(EXPECTED, (traits::is_base_of_ignore_reference<TLEFT, TRIGHT>::value)); \
		EXPECT_EQ(EXPECTED, (traits::is_base_of_ignore_reference<TLEFT, TRIGHT&>::value)); \
		EXPECT_EQ(EXPECTED, (traits::is_base_of_ignore_reference<TLEFT, TRIGHT&&>::value)); \
		\
		EXPECT_EQ(EXPECTED, (traits::is_base_of_ignore_reference_v<TLEFT, TRIGHT>)); \
		EXPECT_EQ(EXPECTED, (traits::is_base_of_ignore_reference_v<TLEFT, TRIGHT&>)); \
		EXPECT_EQ(EXPECTED, (traits::is_base_of_ignore_reference_v<TLEFT, TRIGHT&&>)); \
	} while (false)

	TEST(TEST_CLASS, IsBaseOfIgnoreReferenceReturnsTrueWhenLeftStrippedOfReferenceIsDerivedFromBase) {
		// Assert: (Base, Base) with any reference qualifiers is true
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

		EXPECT_FALSE((traits::is_base_of_ignore_reference_v<Base&, Base>));
		EXPECT_FALSE((traits::is_base_of_ignore_reference_v<Base&&, Base>));
	}

	// endregion

	// region is_type_expression

	namespace {
		template<typename T, typename = void>
		struct FooTagger {
			static constexpr auto Tag = 1u;
		};

		template<typename T>
		struct FooTagger<T, typename traits::is_type_expression<typename T::Foo>::type> {
			static constexpr auto Tag = 2u;
		};

		template<typename T, typename = void>
		struct FooTagger2 {
			static constexpr auto Tag = 1u;
		};

		template<typename T>
		struct FooTagger2<T, traits::is_type_expression_t<typename T::Foo>> {
			static constexpr auto Tag = 2u;
		};
	}

	TEST(TEST_CLASS, IsTypeExpressionCanBeUsedToConditionallySelectTypeBasedOnPresenceOfSubTypeAlias) {
		// Arrange:
		struct FooContainer { using Foo = int; };
		struct OtherContainer {};

		// Assert: FooTagger::Tag is 2u iff T contains a `Foo` type alias
		EXPECT_EQ(2u, static_cast<uint32_t>(FooTagger<FooContainer>::Tag));
		EXPECT_EQ(1u, static_cast<uint32_t>(FooTagger<OtherContainer>::Tag));

		EXPECT_EQ(2u, static_cast<uint32_t>(FooTagger2<FooContainer>::Tag));
		EXPECT_EQ(1u, static_cast<uint32_t>(FooTagger2<OtherContainer>::Tag));
	}

	// endregion

	// region is_template_specialization

	namespace {
		template<typename T, template<typename...> typename TBase>
		void AssertIsTemplateSpecialization(size_t lineNumber) {
			// Assert: only non const variants are supported
			EXPECT_TRUE((traits::is_template_specialization<T, TBase>::value)) << lineNumber;
			EXPECT_TRUE((traits::is_template_specialization_v<T, TBase>)) << lineNumber;
		}

		template<typename T, template<typename...> typename TBase>
		void AssertIsNotTemplateSpecialization(size_t lineNumber) {
			// Assert: only non const variants are supported
			EXPECT_FALSE((traits::is_template_specialization<T, TBase>::value)) << lineNumber;
			EXPECT_FALSE((traits::is_template_specialization_v<T, TBase>)) << lineNumber;
		}
	}

	TEST(TEST_CLASS, IsTemplateSpecializationReturnsTrueOnlyWhenTypeIsATemplateSpecialization) {
		// Assert: byte arrays
		AssertIsTemplateSpecialization<Address, ByteArray>(__LINE__);
		AssertIsTemplateSpecialization<Key, ByteArray>(__LINE__);
		AssertIsTemplateSpecialization<Hash512, ByteArray>(__LINE__);

		// - catapult scalars
		AssertIsTemplateSpecialization<Amount, BaseValue>(__LINE__);
		AssertIsTemplateSpecialization<Height, BaseValue>(__LINE__);
		AssertIsTemplateSpecialization<Difficulty, ClampedBaseValue>(__LINE__);

		// - catapult buffers
		AssertIsTemplateSpecialization<RawBuffer, BasicRawBuffer>(__LINE__);
		AssertIsTemplateSpecialization<MutableRawBuffer, BasicRawBuffer>(__LINE__);

		// - non-matching types
		AssertIsNotTemplateSpecialization<std::vector<int>, ByteArray>(__LINE__);
		AssertIsNotTemplateSpecialization<std::vector<int>, BaseValue>(__LINE__);
		AssertIsNotTemplateSpecialization<int, std::vector>(__LINE__);
	}

	// endregion

	// region is_container

	namespace {
		template<typename T>
		void AssertIsContainer(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_TRUE(traits::is_container<T>::value) << lineNumber;
			EXPECT_TRUE(traits::is_container<const T>::value) << lineNumber;

			EXPECT_TRUE(traits::is_container_v<T>) << lineNumber;
			EXPECT_TRUE(traits::is_container_v<const T>) << lineNumber;
		}

		template<typename T>
		void AssertIsNotContainer(size_t lineNumber) {
			// Assert: both const and non const variants
			EXPECT_FALSE(traits::is_container<T>::value) << lineNumber;
			EXPECT_FALSE(traits::is_container<const T>::value) << lineNumber;

			EXPECT_FALSE(traits::is_container_v<T>) << lineNumber;
			EXPECT_FALSE(traits::is_container_v<const T>) << lineNumber;
		}
	}

	TEST(TEST_CLASS, IsContainerReturnsTrueOnlyForContainersAndContainerLikeTypes) {
		// Assert: stl containers
		AssertIsContainer<std::array<uint8_t, 10>>(__LINE__);
		AssertIsContainer<std::vector<uint8_t>>(__LINE__);
		AssertIsContainer<std::vector<uint64_t>>(__LINE__);
		AssertIsContainer<std::set<uint64_t>>(__LINE__);
		AssertIsContainer<std::unordered_set<uint64_t>>(__LINE__);
		AssertIsContainer<std::map<uint64_t, int>>(__LINE__);

		// - catapult containers
		AssertIsContainer<Key>(__LINE__);
		AssertIsContainer<UnresolvedAddress>(__LINE__);

		// - scalar types and pointers
		AssertIsNotContainer<int>(__LINE__);
		AssertIsNotContainer<void*>(__LINE__);
		AssertIsNotContainer<std::shared_ptr<int>>(__LINE__);
		AssertIsNotContainer<std::unique_ptr<int>>(__LINE__);

		// - catapult scalars
		AssertIsNotContainer<Height>(__LINE__);
		AssertIsNotContainer<Difficulty>(__LINE__);
		AssertIsNotContainer<ClampedBaseValue<int32_t, void>>(__LINE__);
		AssertIsNotContainer<ImmutableValue<uint64_t>>(__LINE__);

		// - catapult structs
		AssertIsNotContainer<RawBuffer>(__LINE__);
		AssertIsNotContainer<MutableRawString>(__LINE__);
	}

	// endregion
}}
