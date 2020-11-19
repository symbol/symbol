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
		EXPECT_EQ(1, pSharedInt.use_count());
		EXPECT_EQ(pRawInt, pSharedInt.get());
	}

	// endregion

	// region memcpy_cond

	TEST(TEST_CLASS, ConditionalMemcpyBypassesZeroSizeOperation) {
		// Arrange:
		std::array<uint8_t, 4> buffer;

		// Act + Assert:
		EXPECT_NO_THROW(memcpy_cond(nullptr, nullptr, 0));
		EXPECT_NO_THROW(memcpy_cond(nullptr, &buffer[0], 0));
		EXPECT_NO_THROW(memcpy_cond(&buffer[0], nullptr, 0));
	}

	TEST(TEST_CLASS, ConditionalMemcpyProcessesNonzeroSizeOperation) {
		// Arrange:
		std::array<uint8_t, 4> src{ { 9, 5, 7, 6 } };
		std::array<uint8_t, 4> dest{ { 1, 2, 3, 4 } };

		// Act:
		memcpy_cond(&dest[0], &src[0], 2);

		// Assert:
		std::array<uint8_t, 4> expected{ { 9, 5, 3, 4 } };
		EXPECT_EQ(expected, dest);
	}

	// endregion
}}
