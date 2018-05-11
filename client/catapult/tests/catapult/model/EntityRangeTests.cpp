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

#include "catapult/model/EntityRange.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/IteratorTestTraits.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS EntityRangeTests

	namespace {
		// region  Test Buffers

		constexpr std::array<uint8_t, 4> Single_Entity_Buffer{ { 0x00, 0x11, 0x22, 0x33 } };

		std::vector<uint32_t> GetExpectedSingleEntityBufferValues() {
			return { 0x33221100 };
		}

		constexpr std::array<uint8_t, 12> Multi_Entity_Buffer{{
			0x00, 0x11, 0x22, 0x33,
			0xFF, 0xDD, 0xBB, 0x99,
			0x76, 0x98, 0x12, 0x34,
		}};

		std::vector<uint32_t> GetExpectedMultiEntityBufferValues() {
			return { 0x33221100, 0x99BBDDFF, 0x34129876 };
		}

		// endregion

		template<typename TValue>
		const auto& DerefValue(const TValue& value) {
			return value;
		}

		const auto& DerefValue(const std::unique_ptr<Block>& pBlock) {
			return *pBlock;
		}

		template<typename TIterator>
		void AssertEmptyIteration(TIterator begin, TIterator end) {
			// Assert:
			EXPECT_EQ(end, begin);
		}

		template<typename TIterator, typename TValuesVector>
		void AssertIteration(TIterator begin, TIterator end, const TValuesVector& expected) {
			// Assert:
			auto iter = begin;
			for (auto i = 0u; i < expected.size(); ++i) {
				EXPECT_EQ(*iter, DerefValue(expected[i])) << "at index " << i;
				++iter;
			}

			EXPECT_EQ(end, iter);
		}

		template<typename TRange>
		void AssertDifferentBackingMemory(const TRange& lhs, const TRange& rhs) {
			// Assert: the backing memory of the first elements should be different
			//         (do not use data() because this function is also called with non-contiguous data ranges)
			EXPECT_NE(&*lhs.cbegin(), &*rhs.cbegin());
		}

		template<typename TRange>
		void AssertEmptyRange(const TRange& range) {
			// Assert: the range is empty
			EXPECT_TRUE(range.empty());
			EXPECT_EQ(0u, range.size());
			EXPECT_EQ(0u, range.totalSize());

			// - non-const and const iteration should produce the same results
			auto& mutableRange = const_cast<TRange&>(range);
			AssertEmptyIteration(mutableRange.begin(), mutableRange.end());
			AssertEmptyIteration(range.begin(), range.end());
			AssertEmptyIteration(range.cbegin(), range.cend());

			// - data pointers are not accessible
			EXPECT_FALSE(!!mutableRange.data());
			EXPECT_FALSE(!!range.data());
		}

		void AssertBasicNonEmptyRange(const EntityRange<uint32_t>& range, const std::vector<uint32_t>& expected, size_t excessSize = 0) {
			// Assert: the range has the correct size
			EXPECT_FALSE(range.empty());
			EXPECT_EQ(expected.size(), range.size());
			EXPECT_EQ(expected.size() * sizeof(uint32_t) + excessSize, range.totalSize());

			// Assert: non-const and const iteration should produce the same results
			auto& mutableRange = const_cast<EntityRange<uint32_t>&>(range);
			AssertIteration(mutableRange.begin(), mutableRange.end(), expected);
			AssertIteration(range.begin(), range.end(), expected);
			AssertIteration(range.cbegin(), range.cend(), expected);
		}

		void AssertNonEmptyRange(const EntityRange<uint32_t>& range, const std::vector<uint32_t>& expected, size_t excessSize = 0) {
			// Assert:
			AssertBasicNonEmptyRange(range, expected, excessSize);

			// - data pointers should point to first element
			auto& mutableRange = const_cast<EntityRange<uint32_t>&>(range);
			EXPECT_EQ(&*range.begin(), mutableRange.data());
			EXPECT_EQ(&*range.begin(), range.data());
		}

		void AssertNonEmptyRangeWithNonContiguousData(
				const EntityRange<uint32_t>& range,
				const std::vector<uint32_t>& expected,
				size_t excessSize = 0) {
			// Assert:
			AssertBasicNonEmptyRange(range, expected, excessSize);

			// - data pointers are not accessible
			auto& mutableRange = const_cast<EntityRange<uint32_t>&>(range);
			EXPECT_THROW(mutableRange.data(), catapult_runtime_error);
			EXPECT_THROW(range.data(), catapult_runtime_error);
		}
	}

	// region empty (default)

	TEST(TEST_CLASS, CanCreateEmptyRange) {
		// Act:
		EntityRange<uint32_t> range;

		// Assert:
		AssertEmptyRange(range);
	}

	TEST(TEST_CLASS, CanCopyEmptyRange) {
		// Act:
		EntityRange<uint32_t> original;
		auto range = EntityRange<uint32_t>::CopyRange(original);

		// Assert:
		AssertEmptyRange(original);
		AssertEmptyRange(range);
	}

	TEST(TEST_CLASS, CanExtractEntitiesFromEmptyRange) {
		// Arrange:
		EntityRange<uint32_t> range;

		// Act:
		auto entities = EntityRange<uint32_t>::ExtractEntitiesFromRange(std::move(range));

		// Sanity:
		AssertEmptyRange(range);

		// Assert: zero entities were extracted
		EXPECT_TRUE(entities.empty());
	}

	// endregion

	// region move

	TEST(TEST_CLASS, MoveConstructorEmptiesSource) {
		// Arrange:
		auto source = EntityRange<uint32_t>::PrepareFixed(3);

		// Sanity:
		EXPECT_FALSE(source.empty());

		// Act:
		EntityRange<uint32_t> dest(std::move(source));

		// Assert:
		AssertEmptyRange(source);
		EXPECT_FALSE(dest.empty());
	}

	TEST(TEST_CLASS, MoveAssignmentEmptiesSource) {
		// Arrange:
		auto source = EntityRange<uint32_t>::PrepareFixed(3);

		// Sanity:
		EXPECT_FALSE(source.empty());

		// Act:
		EntityRange<uint32_t> dest;
		const auto& result = (dest = std::move(source));

		// Assert:
		AssertEmptyRange(source);
		EXPECT_FALSE(dest.empty());
		EXPECT_EQ(&dest, &result);
	}

	// endregion

	// region single buffer, uninitialized (PrepareFixed)

	TEST(TEST_CLASS, CanCreateRangeAroundUninitializedMemory) {
		// Act:
		auto range = EntityRange<uint32_t>::PrepareFixed(3);

		// Assert:
		EXPECT_FALSE(range.empty());
		EXPECT_EQ(3u, range.size());
		EXPECT_EQ(3u * sizeof(uint32_t), range.totalSize());
	}

	TEST(TEST_CLASS, CanWriteToUnderlyingRangeMemoryUsingDataPointer) {
		// Arrange:
		uint8_t* pRangeData;
		auto range = EntityRange<uint32_t>::PrepareFixed(3, &pRangeData);

		// Act:
		std::memcpy(pRangeData, Multi_Entity_Buffer.data(), Multi_Entity_Buffer.size());

		// Assert:
		EXPECT_FALSE(range.empty());
		EXPECT_EQ(3u, range.size());
		EXPECT_EQ(3u * sizeof(uint32_t), range.totalSize());
		AssertNonEmptyRange(range, GetExpectedMultiEntityBufferValues());
	}

	// endregion

	// region single buffer, initialized (CopyFixed, CopyVariable)

	namespace {
		struct FixedTraits {
			template<typename TContainer>
			static auto CreateRange(const TContainer& buffer, std::vector<size_t>&& offsets) {
				return EntityRange<uint32_t>::CopyFixed(buffer.data(), offsets.size());
			}
		};

		struct VariableTraits {
			template<typename TContainer>
			static auto CreateRange(const TContainer& buffer, std::vector<size_t>&& offsets) {
				return EntityRange<uint32_t>::CopyVariable(buffer.data(), buffer.size(), offsets);
			}
		};
	}

#define VARIABLE_OR_FIXED_FACTORY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Fixed) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<FixedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Variable) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VariableTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	VARIABLE_OR_FIXED_FACTORY_TEST(CanCreateRangeAroundSingleEntityBuffer) {
		// Act:
		auto range = TTraits::CreateRange(Single_Entity_Buffer, { 0 });

		// Assert:
		AssertNonEmptyRange(range, GetExpectedSingleEntityBufferValues());
	}

	VARIABLE_OR_FIXED_FACTORY_TEST(CanCopyRangeAroundSingleEntityBuffer) {
		// Act:
		auto original = TTraits::CreateRange(Single_Entity_Buffer, { 0 });
		auto range = EntityRange<uint32_t>::CopyRange(original);

		// Assert:
		AssertNonEmptyRange(original, GetExpectedSingleEntityBufferValues());
		AssertNonEmptyRange(range, GetExpectedSingleEntityBufferValues());
		AssertDifferentBackingMemory(original, range);
	}

	VARIABLE_OR_FIXED_FACTORY_TEST(CanCreateRangeAroundMultipleEntityBuffer) {
		// Act:
		auto range = TTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });

		// Assert:
		AssertNonEmptyRange(range, GetExpectedMultiEntityBufferValues());
	}

	VARIABLE_OR_FIXED_FACTORY_TEST(CanCopyRangeAroundMultipleEntityBuffer) {
		// Act:
		auto original = TTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });
		auto range = EntityRange<uint32_t>::CopyRange(original);

		// Assert:
		AssertNonEmptyRange(original, GetExpectedMultiEntityBufferValues());
		AssertNonEmptyRange(range, GetExpectedMultiEntityBufferValues());
		AssertDifferentBackingMemory(original, range);
	}

	namespace {
		template<typename TContainer>
		void AssertEntities(const TContainer& expectedEntities, const std::vector<std::shared_ptr<uint32_t>>& entities) {
			ASSERT_EQ(expectedEntities.size(), entities.size());
			for (auto i = 0u; i < expectedEntities.size(); ++i)
				EXPECT_EQ(expectedEntities[i], *entities[i]) << "entity at " << i;
		}
	}

	TEST(TEST_CLASS, CanExtractEntitiesFromMultipleEntityBufferRange) {
		// Act:
		auto range = EntityRange<uint32_t>::CopyVariable(Multi_Entity_Buffer.data(), Multi_Entity_Buffer.size(), { 0, 4, 8 });
		auto entities = EntityRange<uint32_t>::ExtractEntitiesFromRange(std::move(range));

		// Sanity:
		AssertEmptyRange(range);

		// Assert:
		AssertEntities(GetExpectedMultiEntityBufferValues(), entities);
	}

	// endregion

	// region overlay (variable) buffer

	namespace {
		constexpr std::array<uint8_t, 16> Multi_Entity_Overlay_Buffer{{
			0x00, 0x11, 0x22, 0x33,
			0xFF, 0xDD, 0xBB, 0x99,
			0x76, 0x98, 0x12, 0x34,
			0xAA, 0xBB, 0xCC, 0xDD,
		}};

		std::vector<uint32_t> GetExpectedMultiEntityOverlayBufferValues() {
			return { 0xDDFF3322, 0x987699BB };
		}
	}

	TEST(TEST_CLASS, CanCreateOverlayRangeAroundPartOfMultipleEntityBuffer) {
		// Act:
		auto range = EntityRange<uint32_t>::CopyVariable(Multi_Entity_Overlay_Buffer.data(), Multi_Entity_Overlay_Buffer.size(), { 2, 6 });

		// Assert: the range is 8 bytes larger than expected (only 2 uint32_t in a 16 byte buffer are used)
		AssertNonEmptyRange(range, GetExpectedMultiEntityOverlayBufferValues(), 8);
	}

	TEST(TEST_CLASS, CanCopyOverlayRangeAroundPartOfMultipleEntityBuffer) {
		// Act:
		auto original = EntityRange<uint32_t>::CopyVariable(
				Multi_Entity_Overlay_Buffer.data(),
				Multi_Entity_Overlay_Buffer.size(),
				{ 2, 6 });
		auto range = EntityRange<uint32_t>::CopyRange(original);

		// Assert: the range is 8 bytes larger than expected (only 2 uint32_t in a 16 byte buffer are used)
		AssertNonEmptyRange(original, GetExpectedMultiEntityOverlayBufferValues(), 8);
		AssertNonEmptyRange(range, GetExpectedMultiEntityOverlayBufferValues(), 8);
		AssertDifferentBackingMemory(original, range);
	}

	TEST(TEST_CLASS, CanExtractEntitiesFromOverlayRangeAroundPartOfMultipleEntityBuffer) {
		// Act:
		auto range = EntityRange<uint32_t>::CopyVariable(Multi_Entity_Overlay_Buffer.data(), Multi_Entity_Overlay_Buffer.size(), { 2, 6 });
		auto entities = EntityRange<uint32_t>::ExtractEntitiesFromRange(std::move(range));

		// Sanity:
		AssertEmptyRange(range);

		// Assert:
		AssertEntities(GetExpectedMultiEntityOverlayBufferValues(), entities);
	}

	// endregion

	// region single entity

	namespace {
		void AssertSingleBlockRange(const BlockRange& range, const Block& expectedBlock) {
			auto& mutableRange = const_cast<BlockRange&>(range);

			// Assert:
			EXPECT_FALSE(range.empty());
			ASSERT_EQ(1u, range.size());
			EXPECT_EQ(expectedBlock.Size, range.totalSize());

			EXPECT_EQ(expectedBlock, *mutableRange.begin());
			EXPECT_EQ(expectedBlock, *range.begin());
			EXPECT_EQ(expectedBlock, *range.cbegin());
		}
	}

	TEST(TEST_CLASS, CanCreateRangeAroundSingleVerifiableEntity) {
		// Arrange:
		auto pBlock = test::NonEmptyBlockPolicy::Create();
		auto pBlockCopy = test::CopyBlock(*pBlock);

		// Act:
		auto range = BlockRange::FromEntity(std::move(pBlock));

		// Sanity:
		EXPECT_FALSE(!!pBlock);

		// Assert:
		AssertSingleBlockRange(range, *pBlockCopy);
	}

	TEST(TEST_CLASS, CanCopyRangeAroundSingleVerifiableEntity) {
		// Arrange:
		auto pBlock = test::NonEmptyBlockPolicy::Create();
		auto pBlockCopy = test::CopyBlock(*pBlock);

		// Act:
		auto original = BlockRange::FromEntity(std::move(pBlock));
		auto range = BlockRange::CopyRange(original);

		// Sanity:
		EXPECT_FALSE(!!pBlock);

		// Assert:
		AssertSingleBlockRange(original, *pBlockCopy);
		AssertSingleBlockRange(range, *pBlockCopy);
		AssertDifferentBackingMemory(original, range);
	}

	TEST(TEST_CLASS, CanExtractEntitiesFromSingleVerifiableEntityRange) {
		// Arrange:
		auto pBlock = test::NonEmptyBlockPolicy::Create();
		auto pBlockRaw = pBlock.get();
		auto pBlockCopy = test::CopyBlock(*pBlock);

		// Act:
		auto range = BlockRange::FromEntity(std::move(pBlock));
		auto blocks = BlockRange::ExtractEntitiesFromRange(std::move(range));

		// Sanity:
		AssertEmptyRange(range);

		// Assert: the original (seed) pointer should be returned
		ASSERT_EQ(1u, blocks.size());
		EXPECT_EQ(*pBlockCopy, *blocks[0]);
		EXPECT_EQ(pBlockRaw, blocks[0].get());
	}

	// endregion

	// region multi buffer (merge)

	TEST(TEST_CLASS, CanMergeZeroEntityRanges) {
		// Arrange
		std::vector<EntityRange<uint32_t>> ranges;

		// Act:
		auto mergedRange = EntityRange<uint32_t>::MergeRanges(std::move(ranges));

		// Assert:
		AssertEmptyRange(mergedRange);
	}

	TEST(TEST_CLASS, CanMergeSingleEmptyEntityRange) {
		// Arrange
		std::vector<EntityRange<uint32_t>> ranges;
		ranges.push_back(EntityRange<uint32_t>());

		// Act:
		auto mergedRange = EntityRange<uint32_t>::MergeRanges(std::move(ranges));

		// Assert:
		AssertEmptyRange(mergedRange);
	}

	TEST(TEST_CLASS, CanMergeMultipleEmptyEntityRanges) {
		// Arrange
		std::vector<EntityRange<uint32_t>> ranges;
		for (auto i = 0u; i < 10; ++i)
			ranges.push_back(EntityRange<uint32_t>());

		// Act:
		auto mergedRange = EntityRange<uint32_t>::MergeRanges(std::move(ranges));

		// Assert:
		AssertEmptyRange(mergedRange);
	}

	VARIABLE_OR_FIXED_FACTORY_TEST(CanMergeSingleNonEmptyEntityRange) {
		// Arrange
		std::vector<EntityRange<uint32_t>> ranges;
		auto range = TTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });
		ranges.push_back(std::move(range));

		// Act:
		auto mergedRange = EntityRange<uint32_t>::MergeRanges(std::move(ranges));

		// Assert:
		AssertNonEmptyRangeWithNonContiguousData(mergedRange, GetExpectedMultiEntityBufferValues());
	}

	namespace {
		constexpr std::array<uint8_t, 12> Multi_Entity_Buffer_2{ {
			0x23, 0x34, 0x45, 0x56,
			0x19, 0x28, 0x37, 0x46,
			0x24, 0x35, 0x46, 0x57,
		} };

		std::vector<uint32_t> GetExpectedMultiEntityBuffer2Values() {
			return { 0x56453423, 0x46372819, 0x57463524 };
		}
	}

	VARIABLE_OR_FIXED_FACTORY_TEST(CanMergeMultipleNonEmptyEntityRanges) {
		// Arrange
		std::vector<EntityRange<uint32_t>> ranges;
		auto range1 = TTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });
		auto range2 = TTraits::CreateRange(Single_Entity_Buffer, { 0 });
		auto range3 = TTraits::CreateRange(Multi_Entity_Buffer_2, { 0, 4, 8 });
		ranges.push_back(std::move(range1));
		ranges.push_back(std::move(range2));
		ranges.push_back(std::move(range3));

		// Act:
		auto mergedRange = EntityRange<uint32_t>::MergeRanges(std::move(ranges));

		// Assert:
		auto expectedEntities = GetExpectedMultiEntityBufferValues();
		auto expectedEntities2 = GetExpectedSingleEntityBufferValues();
		auto expectedEntities3 = GetExpectedMultiEntityBuffer2Values();
		expectedEntities.insert(expectedEntities.end(), expectedEntities2.begin(), expectedEntities2.end());
		expectedEntities.insert(expectedEntities.end(), expectedEntities3.begin(), expectedEntities3.end());

		AssertNonEmptyRangeWithNonContiguousData(mergedRange, expectedEntities);
	}

	VARIABLE_OR_FIXED_FACTORY_TEST(CanMergeEmptyAndNonEmptyEntityRanges) {
		// Arrange
		std::vector<EntityRange<uint32_t>> ranges;
		ranges.push_back(TTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 }));
		for (auto i = 0u; i < 5; ++i)
			ranges.push_back(EntityRange<uint32_t>());

		ranges.push_back(TTraits::CreateRange(Single_Entity_Buffer, { 0 }));

		// Act:
		auto mergedRange = EntityRange<uint32_t>::MergeRanges(std::move(ranges));

		// Assert:
		auto expectedEntities = GetExpectedMultiEntityBufferValues();
		auto expectedEntities2 = GetExpectedSingleEntityBufferValues();
		expectedEntities.insert(expectedEntities.end(), expectedEntities2.begin(), expectedEntities2.end());

		AssertNonEmptyRangeWithNonContiguousData(mergedRange, expectedEntities);
	}

	namespace {
		void AssertMultiBlockRange(const std::vector<std::unique_ptr<Block>>& expectedBlocks, const BlockRange& range) {
			// Assert:
			EXPECT_FALSE(range.empty());
			ASSERT_EQ(expectedBlocks.size(), range.size());
			EXPECT_EQ(sizeof(Block) * expectedBlocks.size(), range.totalSize());

			// Assert: non-const and const iteration should produce the same results
			auto& mutableRange = const_cast<BlockRange&>(range);
			AssertIteration(mutableRange.begin(), mutableRange.end(), expectedBlocks);
			AssertIteration(range.begin(), range.end(), expectedBlocks);
			AssertIteration(range.cbegin(), range.cend(), expectedBlocks);
		}

		template<typename TFunc>
		void RunHeterogeneousMergeRangesTest(TFunc func) {
			// Arrange: merge all types of ranges (single-buffer, single-entity, multi-buffer)
			std::vector<std::unique_ptr<Block>> blocks;
			for (auto i = 0u; i < 6; ++i)
				blocks.push_back(test::GenerateEmptyRandomBlock());

			std::vector<BlockRange> ranges;
			ranges.push_back(test::CreateEntityRange({ blocks[0].get() })); // single-buffer
			ranges.push_back(BlockRange::FromEntity(test::CopyBlock(*blocks[1]))); // single-entity
			ranges.push_back(test::CreateEntityRange(std::vector<const Block*>{ blocks[2].get(), blocks[3].get() })); // single-buffer

			std::vector<BlockRange> subRanges;
			subRanges.push_back(test::CreateEntityRange({ blocks[4].get() })); // single-buffer
			subRanges.push_back(BlockRange::FromEntity(test::CopyBlock(*blocks[5]))); // single-entity
			ranges.push_back(BlockRange::MergeRanges(std::move(subRanges))); // multi-buffer

			// Act:
			auto mergedRange = BlockRange::MergeRanges(std::move(ranges));
			func(blocks, mergedRange);
		}
	}

	TEST(TEST_CLASS, CanMergeRangesComposedOfDifferentSubRangeTypes) {
		// Arrange:
		RunHeterogeneousMergeRangesTest([](const auto& blocks, const auto& mergedRange) {
				// Assert:
				AssertMultiBlockRange(blocks, mergedRange);
		});
	}

	TEST(TEST_CLASS, CanCopyMergedRange) {
		// Arrange:
		RunHeterogeneousMergeRangesTest([](const auto& blocks, const auto& mergedRange) {
			// Act:
			auto rangeCopy = BlockRange::CopyRange(mergedRange);

			// Assert:
			AssertMultiBlockRange(blocks, mergedRange);
			AssertMultiBlockRange(blocks, rangeCopy);
			AssertDifferentBackingMemory(mergedRange, rangeCopy);
		});
	}

	TEST(TEST_CLASS, CanExtractEntitiesFromMergedRange) {
		// Arrange:
		RunHeterogeneousMergeRangesTest([](const auto& blocks, auto& mergedRange) {
			// Act:
			auto extractedBlocks = BlockRange::ExtractEntitiesFromRange(std::move(mergedRange));

			// Sanity:
			AssertEmptyRange(mergedRange);

			// Assert:
			ASSERT_EQ(blocks.size(), extractedBlocks.size());
			for (auto i = 0u; i < extractedBlocks.size(); ++i)
				EXPECT_EQ(*blocks[i], *extractedBlocks[i]) << "block at " << i;
		});
	}

	// endregion

	// region iterators

#define ITERATOR_BASED_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_BeginEnd) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BeginEndTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_BeginEndConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BeginEndConstTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_CBeginCEnd) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::CBeginCEndTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ITERATOR_BASED_BASED_TEST(CanIterateOverMultipleEntitiesWithPostfixOperator) {
		// Arrange:
		auto range = FixedTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });

		// Act + Assert:
		auto iter = TTraits::begin(range);
		EXPECT_EQ(0x33221100u, (*iter++));
		EXPECT_EQ(0x99BBDDFFu, (*iter++));
		EXPECT_EQ(0x34129876u, (*iter++));
		EXPECT_EQ(iter, TTraits::end(range));
	}

	ITERATOR_BASED_BASED_TEST(CanIterateOverMultipleEntitiesWithPrefixOperator) {
		// Arrange:
		auto range = FixedTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });

		// Act + Assert:
		auto iter = TTraits::begin(range);
		EXPECT_EQ(0x33221100u, *iter);
		++iter;
		EXPECT_EQ(0x99BBDDFFu, *iter);
		++iter;
		EXPECT_EQ(0x34129876u, *iter);
		++iter;
		EXPECT_EQ(iter, TTraits::end(range));
	}

	ITERATOR_BASED_BASED_TEST(CanReverseIterateOverMultipleEntitiesWithPrefixOperator) {
		// Arrange:
		auto range = FixedTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });

		// Act + Assert:
		auto iter = TTraits::end(range);
		EXPECT_EQ(0x34129876u, (*--iter));
		EXPECT_EQ(0x99BBDDFFu, (*--iter));
		EXPECT_EQ(0x33221100u, (*--iter));
		EXPECT_EQ(iter, TTraits::begin(range));
	}

	ITERATOR_BASED_BASED_TEST(CanReverseIterateOverMultipleEntitiesWithPostfixOperator) {
		// Arrange:
		auto range = FixedTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });

		// Act + Assert:
		auto iter = TTraits::end(range);
		iter--;
		EXPECT_EQ(0x34129876u, *iter);
		iter--;
		EXPECT_EQ(0x99BBDDFFu, *iter);
		iter--;
		EXPECT_EQ(0x33221100u, *iter);
		EXPECT_EQ(iter, TTraits::begin(range));
	}

	ITERATOR_BASED_BASED_TEST(CanDereferenceIterator) {
		// Arrange:
		auto range = FixedTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });
		auto iter = TTraits::begin(range);
		++iter;

		// Assert:
		EXPECT_EQ(0x99BBDDFFu, *iter);
		EXPECT_EQ(0x99BBDDFFu, *(iter.operator->()));

		const auto constIter = iter;
		EXPECT_EQ(0x99BBDDFFu, *constIter);
		EXPECT_EQ(0x99BBDDFFu, *(constIter.operator->()));
	}

	ITERATOR_BASED_BASED_TEST(BeginEndIteratorsBasedOnSameContainerAreEqual) {
		// Arrange:
		auto range = FixedTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });

		// Act + Assert:
		EXPECT_EQ(TTraits::begin(range), TTraits::begin(range));
		EXPECT_EQ(TTraits::end(range), TTraits::end(range));
		EXPECT_NE(TTraits::begin(range), TTraits::end(range));
	}

	ITERATOR_BASED_BASED_TEST(CanCheckArbitraryIteratorsForEquality) {
		// Arrange:
		auto range = FixedTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });
		auto iter = TTraits::begin(range);
		++iter;

		// Act + Assert:
		EXPECT_NE(TTraits::begin(range), iter);
		EXPECT_EQ(++TTraits::begin(range), iter);
		EXPECT_NE(TTraits::end(range), iter);
	}

	namespace {
		template<typename TIterator>
		bool IsIteratorPointeeConst(TIterator iterator) {
			*iterator; // use iterator to workaround vs warning
			return std::is_const<typename std::remove_reference<decltype(*iterator)>::type>::value;
		}
	}

	TEST(TEST_CLASS, IteratorConstnessIsCorrect) {
		// Arrange:
		auto range = FixedTraits::CreateRange(Multi_Entity_Buffer, { 0, 4, 8 });
		const auto& crange = range;

		// Assert:
		EXPECT_TRUE(IsIteratorPointeeConst(range.cbegin()));
		EXPECT_FALSE(IsIteratorPointeeConst(range.begin()));

		EXPECT_TRUE(IsIteratorPointeeConst(crange.cbegin()));
		EXPECT_TRUE(IsIteratorPointeeConst(crange.begin()));
	}

	// endregion

	// region FindFirstDifferenceIndex

	namespace {
		template<typename TEntity>
		void AssertDifferenceIndex(const EntityRange<TEntity>& range1, const EntityRange<TEntity>& range2, size_t expectedIndex) {
			// Assert:
			EXPECT_EQ(expectedIndex, FindFirstDifferenceIndex(range1, range2));
			EXPECT_EQ(expectedIndex, FindFirstDifferenceIndex(range2, range1));
		}
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanCompareEmptyRanges) {
		// Arrange:
		EntityRange<uint32_t> range1;
		EntityRange<uint32_t> range2;

		// Assert:
		AssertDifferenceIndex(range1, range2, 0);
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanCompareSameRangeInstance) {
		// Arrange:
		constexpr uint8_t Data[] = { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBB, 0x99, 0x76, 0x98, 0x12, 0x34 };
		auto range = EntityRange<uint16_t>::CopyFixed(Data, 6);

		// Assert:
		AssertDifferenceIndex(range, range, 6);
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanCompareIdenticalRanges) {
		// Arrange:
		constexpr uint8_t Data[] = { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBB, 0x99, 0x76, 0x98, 0x12, 0x34 };
		auto range1 = EntityRange<uint16_t>::CopyFixed(Data, 6);
		auto range2 = EntityRange<uint16_t>::CopyFixed(Data, 6);

		// Assert:
		AssertDifferenceIndex(range1, range2, 6);
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanCompareRangesWithDifferentSizes) {
		// Arrange:
		constexpr uint8_t Data[] = { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBB, 0x99, 0x76, 0x98, 0x12, 0x34 };
		auto range1 = EntityRange<uint16_t>::CopyFixed(Data, 6);
		auto range2 = EntityRange<uint16_t>::CopyFixed(Data, 4);

		// Assert:
		AssertDifferenceIndex(range1, range2, 4);
	}

	TEST(TEST_CLASS, FindFirstDifferenceIndexCanCompareRangesWithDifferentContents) {
		// Arrange:
		constexpr uint8_t Data1[] = { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBB, 0x99, 0x76, 0x98, 0x12, 0x34 };
		constexpr uint8_t Data2[] = { 0x00, 0x11, 0x22, 0x33, 0xFF, 0xDD, 0xBC, 0x99, 0x76, 0x98, 0x12, 0x34 };
		auto range1 = EntityRange<uint16_t>::CopyFixed(Data1, 6);
		auto range2 = EntityRange<uint16_t>::CopyFixed(Data2, 6);

		// Assert:
		AssertDifferenceIndex(range1, range2, 3);
	}

	// endregion
}}
