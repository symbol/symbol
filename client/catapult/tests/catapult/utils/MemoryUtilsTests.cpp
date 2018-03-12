#include "catapult/utils/MemoryUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS MemoryUtilsTests

	// region MakeWithSize

	namespace {
		struct Foo {
			uint32_t Alpha;
		};

		struct UniqueTraits {
			template<typename T>
			static auto MakeWithSize(size_t size) {
				return MakeUniqueWithSize<T>(size);
			}
		};

		struct SharedTraits {
			template<typename T>
			static auto MakeWithSize(size_t size) {
				return MakeSharedWithSize<T>(size);
			}
		};
	}

#define POINTER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Unique) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UniqueTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Shared) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SharedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	POINTER_TEST(CannotCreatePointerWithInsufficientSize) {
		// Act + Assert:
		EXPECT_THROW(TTraits::template MakeWithSize<Foo>(sizeof(Foo) - 1), catapult_invalid_argument);
	}

	POINTER_TEST(CanCreatePointerWithMinimumSize) {
		// Act:
		auto pFoo = TTraits::template MakeWithSize<Foo>(sizeof(Foo));

		// Assert:
		EXPECT_TRUE(!!pFoo);
	}

	POINTER_TEST(CanCreatePointerWithCustomSize) {
		// Act:
		auto pFoo = TTraits::template MakeWithSize<Foo>(sizeof(Foo) + 123);

		// Assert:
		EXPECT_TRUE(!!pFoo);
	}

	// endregion

	// region UniqueToShared

	TEST(TEST_CLASS, CanConvertUniquePointerToSharedPointer) {
		// Arrange:
		auto pUniqueInt = std::make_unique<int>(123);
		const auto* pRawInt = pUniqueInt.get();

		// Act:
		auto pSharedInt = UniqueToShared(std::move(pUniqueInt));

		// Assert:
		EXPECT_FALSE(!!pUniqueInt);
		EXPECT_TRUE(!!pSharedInt);
		EXPECT_TRUE(pSharedInt.unique());
		EXPECT_EQ(pRawInt, pSharedInt.get());
	}

	// endregion
}}
