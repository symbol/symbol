#include "catapult/utils/traits/Traits.h"
#include "catapult/types.h"
#include "tests/TestHarness.h"
#include <array>

namespace catapult { namespace utils {

#define TEST_CLASS TraitsTests

	// region is_scalar / is_pod

	namespace {
		template<typename T>
		void AssertIsScalar() {
			// Assert: both const and non const variants
			EXPECT_TRUE(traits::is_scalar<T>::value);
			EXPECT_TRUE(traits::is_scalar<const T>::value);
		}

		template<typename T>
		void AssertIsNotScalar() {
			// Assert: both const and non const variants
			EXPECT_FALSE(traits::is_scalar<T>::value);
			EXPECT_FALSE(traits::is_scalar<const T>::value);
		}

		template<typename T>
		void AssertIsPod() {
			// Assert: both const and non const variants
			EXPECT_TRUE(traits::is_pod<T>::value);
			EXPECT_TRUE(traits::is_pod<const T>::value);
		}

		template<typename T>
		void AssertIsNotPod() {
			// Assert: both const and non const variants
			EXPECT_FALSE(traits::is_pod<T>::value);
			EXPECT_FALSE(traits::is_pod<const T>::value);
		}
	}

	TEST(TEST_CLASS, IsScalarReturnsTrueOnlyForIntegralAndIntegralLikeTypes) {
		// Assert:
		// - basic and pointer types
		AssertIsScalar<int>();
		AssertIsScalar<void*>();
		AssertIsNotScalar<std::shared_ptr<int>>();
		AssertIsNotScalar<std::unique_ptr<int>>();

		// - catapult scalars
		AssertIsScalar<Height>(); // well known base value
		AssertIsScalar<Difficulty>(); // well known clamped base value
		AssertIsScalar<BaseValue<short, void>>(); // arbitrary base value
		AssertIsScalar<ClampedBaseValue<int32_t, void>>(); // arbitrary clamped base value
		AssertIsScalar<ImmutableValue<uint64_t>>(); // arbitrary immutable value

		// - compound types
		AssertIsNotScalar<std::vector<int>>();
		AssertIsNotScalar<std::array<unsigned char, 1>>();
	}

	TEST(TEST_CLASS, IsPodReturnsTrueOnlyForIntegralAndIntegralLikeTypes) {
		// Assert:
		// - basic and pointer types
		AssertIsPod<int>();
		AssertIsNotPod<void*>();
		AssertIsNotPod<std::shared_ptr<int>>();
		AssertIsNotPod<std::unique_ptr<int>>();

		// - catapult scalars
		AssertIsPod<Height>(); // well known base value
		AssertIsPod<Difficulty>(); // well known clamped base value
		AssertIsPod<BaseValue<short, void>>(); // arbitrary base value
		AssertIsPod<ClampedBaseValue<int32_t, void>>(); // arbitrary clamped base value
		AssertIsPod<ImmutableValue<uint64_t>>(); // arbitrary immutable value

		// - compound types
		AssertIsNotPod<std::vector<int>>();
		AssertIsPod<std::array<unsigned char, 1>>();
	}

	// endregion

	// region is_base_of_ignore_reference

	namespace {
		struct Base {
		};

		struct Derived : Base {
		};

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
