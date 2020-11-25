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

#include "catapult/crypto/OpensslMemory.h"
#include "catapult/thread/ThreadGroup.h"
#include "tests/TestHarness.h"
#include <openssl/crypto.h>

namespace catapult { namespace crypto {

#define TEST_CLASS OpensslMemoryTests

	// region FixedSizePool

	namespace {
		constexpr size_t Element_Size = 12;
		constexpr size_t Test_Count = 5;
		using TestPool = FixedSizePool<FixedSizePoolTraits<Element_Size, Element_Size, Test_Count>>;
		using PoolBuffer = std::array<uint8_t, Element_Size * Test_Count>;
	}

	TEST(TEST_CLASS, FixedSizePool_BufferReturnsBeginningOfProvidedBuffer) {
		// Arrange:
		PoolBuffer buffer;
		TestPool pool(buffer.data());

		// Act:
		const auto* pBuffer = pool.buffer();

		// Arrange:
		EXPECT_EQ(buffer.data(), pBuffer);
	}

	TEST(TEST_CLASS, FixedSizePool_CanAllocateSingleElement) {
		// Arrange:
		PoolBuffer buffer;
		TestPool pool(buffer.data());

		// Act:
		const auto* pElement = pool.tryAllocate();

		// Assert:
		EXPECT_EQ(buffer.data(), pool.buffer());
		EXPECT_EQ(buffer.data(), pElement);
	}

	TEST(TEST_CLASS, FixedSizePool_CanAllocateMaxElements) {
		// Arrange:
		PoolBuffer buffer;
		TestPool pool(buffer.data());
		std::vector<uint8_t*> pointers;

		// Act:
		for (auto i = 0u; i < Test_Count; ++i)
			pointers.push_back(pool.tryAllocate());

		// Assert:
		EXPECT_EQ(buffer.data(), pool.buffer());
		for (auto i = 0u; i < Test_Count; ++i)
			EXPECT_EQ(buffer.data() + i * Element_Size, pointers[i]) << i;
	}

	TEST(TEST_CLASS, FixedSizePool_CannotAllocateMoreThanMaxElementsAtOnce) {
		// Arrange:
		PoolBuffer buffer;
		TestPool pool(buffer.data());
		for (auto i = 0u; i < Test_Count; ++i)
			pool.tryAllocate();

		// Act + Assert:
		EXPECT_FALSE(!!pool.tryAllocate());
		EXPECT_FALSE(!!pool.tryAllocate());
	}

	TEST(TEST_CLASS, FixedSizePool_CanAllocateWhenSomeElementsHaveBeenFreed) {
		// Arrange: allocate all elements
		PoolBuffer buffer;
		TestPool pool(buffer.data());
		std::vector<uint8_t*> pointers;
		for (auto i = 0u; i < Test_Count; ++i)
			pointers.push_back(pool.tryAllocate());

		// Sanity:
		EXPECT_EQ(Test_Count, pointers.size());
		EXPECT_FALSE(!!pool.tryAllocate());

		// Act: release some elements
		pool.free(pointers[1]);
		pool.free(pointers[3]);

		// - can allocate 2 more pointers:
		std::vector<uint8_t*> newPointers;
		for (auto i = 0u; i < 2; ++i)
			newPointers.push_back(pool.tryAllocate());

		// Assert:
		for (const auto* pElement : newPointers)
			EXPECT_TRUE(!!pElement);

		// - all pointers exhausted again, cannot allocate more
		EXPECT_FALSE(!!pool.tryAllocate());
		EXPECT_FALSE(!!pool.tryAllocate());
	}

	// endregion

	// region SpecializedOpensslPoolAllocator

	namespace {
		using Allocator = SpecializedOpensslPoolAllocator;

		constexpr auto Size1 = Allocator::Pool1::Unaligned_Element_Size;
		constexpr auto Size2 = Allocator::Pool2::Unaligned_Element_Size;
		constexpr auto Size3 = Allocator::Pool3::Unaligned_Element_Size;

		void AssertValidPoolPointer(const SpecializedOpensslPoolAllocator& allocator, const uint8_t* pElement) {
			EXPECT_TRUE(!!pElement);
			EXPECT_TRUE(allocator.isFromPool(pElement));
		}
	}

#define TRAIT_BASED_ALLOCATOR_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Size1) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Allocator::Pool1>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Size2) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Allocator::Pool2>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Size3) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<Allocator::Pool3>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAIT_BASED_ALLOCATOR_TEST(SpecializedOpensslPoolAllocator_CanAllocateSingleElement) {
		// Arrange:
		Allocator allocator;

		// Act:
		const auto* pElement = allocator.allocate(TTraits::Unaligned_Element_Size);

		// Assert:
		AssertValidPoolPointer(allocator, pElement);
	}

	TRAIT_BASED_ALLOCATOR_TEST(SpecializedOpensslPoolAllocator_IsFromPoolReturnsFalseWhenPointerIsNotFromPool) {
		// Arrange:
		Allocator allocator;

		// Act:
		auto pElement = std::make_unique<uint8_t[]>(TTraits::Unaligned_Element_Size);

		// Assert: valid pointer
		EXPECT_TRUE(!!pElement);
		EXPECT_FALSE(allocator.isFromPool(pElement.get()));
	}

	TEST(TEST_CLASS, SpecializedOpensslPoolAllocator_IsFromPoolReturnsFalseWhenPointerIsOutsidePool) {
		// Arrange:
		Allocator allocator;

		// Act: drain all elements from last pool
		std::vector<uint8_t*> pointers;
		for (auto i = 0u; i < Allocator::Pool3::Count; ++i)
			pointers.push_back(allocator.allocate(Size3));

		// Assert: calculate where next pointer would be
		const auto* pSpecial = pointers.back() + Size3;
		EXPECT_FALSE(allocator.isFromPool(pSpecial));
	}

	TRAIT_BASED_ALLOCATOR_TEST(SpecializedOpensslPoolAllocator_CannotAllocateAfterDraining) {
		// Arrange:
		Allocator allocator;
		std::vector<uint8_t*> pointers;

		// - drain pool
		for (auto i = 0u; i < TTraits::Count; ++i)
			pointers.push_back(allocator.allocate(TTraits::Unaligned_Element_Size));

		// Act:
		const auto* pLastElement = allocator.allocate(TTraits::Unaligned_Element_Size);

		// Assert:
		for (const auto* pElement : pointers)
			AssertValidPoolPointer(allocator, pElement);

		EXPECT_FALSE(!!pLastElement);
	}

	TRAIT_BASED_ALLOCATOR_TEST(SpecializedOpensslPoolAllocator_CopyToCopiesAtMostFixedChunkSize) {
		// Arrange:
		Allocator allocator;
		const auto* pElement = allocator.allocate(TTraits::Unaligned_Element_Size);

		// - prepare destination buffer with two sentinel bytes
		auto buffer = test::GenerateRandomArray<TTraits::Unaligned_Element_Size + 2>();
		buffer[TTraits::Unaligned_Element_Size] = 0xCA;
		buffer[TTraits::Unaligned_Element_Size + 1] = 0xFE;

		// Act:
		allocator.copyTo(buffer.data(), pElement, buffer.size());

		// Assert: buffer matches, sentinel bytes are untouched
		EXPECT_EQ_MEMORY(pElement, buffer.data(), TTraits::Unaligned_Element_Size);
		EXPECT_EQ(0xCAu, buffer[TTraits::Unaligned_Element_Size]);
		EXPECT_EQ(0xFEu, buffer[TTraits::Unaligned_Element_Size + 1]);
	}

	TRAIT_BASED_ALLOCATOR_TEST(SpecializedOpensslPoolAllocator_CopyToCopiesDoesNotOverflow) {
		// Arrange:
		Allocator allocator;
		const auto* pElement = allocator.allocate(TTraits::Unaligned_Element_Size);

		// - prepare destination buffer with sentinel byte
		constexpr auto Short_Buffer_Size = TTraits::Unaligned_Element_Size - 1;
		auto buffer = test::GenerateRandomArray<Short_Buffer_Size>();
		buffer[Short_Buffer_Size - 1] = 0xCA;

		// Act: copy at most TTraits::Size - 2 bytes.
		allocator.copyTo(buffer.data(), pElement, TTraits::Unaligned_Element_Size - 2);

		// Assert: buffer matches, sentinel byte is untouched
		EXPECT_EQ_MEMORY(pElement, buffer.data(), TTraits::Unaligned_Element_Size - 2);
		EXPECT_EQ(0xCAu, buffer[Short_Buffer_Size - 1]);
	}

	TEST(TEST_CLASS, SpecializedOpensslPoolAllocator_CanAllocateSingleElementOfEachSize) {
		// Arrange: shared allocator
		Allocator allocator;

		// Act:
		const auto* pElement1 = allocator.allocate(Size1);
		const auto* pElement2 = allocator.allocate(Size2);
		const auto* pElement3 = allocator.allocate(Size3);

		// Assert:
		AssertValidPoolPointer(allocator, pElement1);
		AssertValidPoolPointer(allocator, pElement2);
		AssertValidPoolPointer(allocator, pElement3);
	}

	TEST(TEST_CLASS, SpecializedOpensslPoolAllocator_CanAllocateMultipleElements_OfDifferentSizes) {
		// Arrange:
		constexpr size_t Sizes[] = { Size1, Size2, Size3 };
		Allocator allocator;
		std::vector<uint8_t*> pointers;

		// Act:
		for (auto i = 0u; i < 13; ++i)
			pointers.push_back(allocator.allocate(Sizes[i % CountOf(Sizes)]));

		// Assert:
		for (const auto* pElement : pointers)
			AssertValidPoolPointer(allocator, pElement);
	}

	namespace {
		auto DrainPool(Allocator& allocator) {
			// Arrange:
			constexpr size_t Sizes[] = { Size1, Size2, Size3 };
			constexpr size_t Counts[] = { Allocator::Pool1::Count, Allocator::Pool2::Count, Allocator::Pool3::Count };

			std::vector<uint8_t*> pointers;
			for (auto i = 0u; i < CountOf(Sizes); ++i) {
				for (auto j = 0u; j < Counts[i]; ++j)
					pointers.push_back(allocator.allocate(Sizes[i]));
			}

			return pointers;
		}
	}

	TEST(TEST_CLASS, SpecializedOpensslPoolAllocator_CanAllocateWhenProperElementHasBeenFreed) {
		// Arrange:
		Allocator allocator;
		auto pointers = DrainPool(allocator);

		// Sanity: can't allocate pointer of any size
		EXPECT_FALSE(!!allocator.allocate(Size1));
		EXPECT_FALSE(!!allocator.allocate(Size2));
		EXPECT_FALSE(!!allocator.allocate(Size3));

		// Act + Assert: free element of Size1, cannot allocate Size2 and Size3, can re-allocate Size1
		allocator.free(pointers[3]);
		EXPECT_FALSE(!!allocator.allocate(Size2));
		EXPECT_FALSE(!!allocator.allocate(Size3));
		AssertValidPoolPointer(allocator, allocator.allocate(Size1));

		// - free element of Size2, cannot allocate Size1 and Size3, can re-allocate Size2
		allocator.free(pointers[13]);
		EXPECT_FALSE(!!allocator.allocate(Size1));
		EXPECT_FALSE(!!allocator.allocate(Size3));
		AssertValidPoolPointer(allocator, allocator.allocate(Size2));

		// - free element of Size3, cannot allocate Size1 and Size2, can re-allocate Size3
		allocator.free(pointers[33]);
		EXPECT_FALSE(!!allocator.allocate(Size1));
		EXPECT_FALSE(!!allocator.allocate(Size2));
		AssertValidPoolPointer(allocator, allocator.allocate(Size3));
	}

	TEST(TEST_CLASS, SpecializedOpensslPoolAllocator_CanAllocateFromMultipleThreads) {
		// Arrange:
		constexpr size_t Sizes[] = { Size1, Size2, Size3 };
		Allocator allocator;
		thread::ThreadGroup threads;
		std::vector<uint8_t*> pointers(25);
		std::atomic_uint counter = 0;

		// Act: allocate 25 pointers of different size
		for (auto i = 0u; i < 5u; ++i) {
			threads.spawn([&]() {
				auto threadId = counter++;
				for (auto j = 0u; j < 5; ++j) {
					auto id = threadId * 5 + j;
					pointers[id] = allocator.allocate(Sizes[id % CountOf(Sizes)]);
				}
			});
		}

		threads.join();

		// Assert:
		for (const auto* pElement : pointers)
			AssertValidPoolPointer(allocator, pElement);
	}

	TEST(TEST_CLASS, SpecializedOpensslPoolAllocator_OnlyReturnsAlignedPointers) {
		// Act:
		Allocator allocator;
		auto pointers = DrainPool(allocator);

		// Assert:
		auto i = 0u;
		for (const auto* pElement : pointers) {
			EXPECT_EQ(0u, reinterpret_cast<uintptr_t>(pElement) % 8) << pElement << " at " << i;
			++i;
		}
	}

	TEST(TEST_CLASS, SpecializedOpensslPoolAllocator_CanFillPoolWithoutOverlappingPointers) {
		// Act:
		Allocator allocator;
		auto pointers = DrainPool(allocator);

		constexpr auto Pool2_Start = Allocator::Pool1::Count;
		constexpr auto Pool3_Start = Allocator::Pool1::Count + Allocator::Pool2::Count;

		// Assert:
		const auto* pPrevious = pointers[0];
		for (auto i = 1u; i < pointers.size(); ++i) {
			const auto* pCurrent = pointers[i];
			auto previousSize = i <= Pool2_Start
					? Allocator::Pool1::Element_Size
					: i <= Pool3_Start ? Allocator::Pool2::Element_Size : Allocator::Pool3::Element_Size;

			EXPECT_LT(pPrevious, pCurrent) << i;
			EXPECT_EQ(pPrevious + previousSize, pCurrent) << i;

			pPrevious = pCurrent;
		}
	}

	// endregion
}}
