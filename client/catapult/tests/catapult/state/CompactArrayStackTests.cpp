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

#include "catapult/state/CompactArrayStack.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS CompactArrayStackTests

	// region test utils

	namespace {
		struct ValuePair {
			uint64_t Value1 = 0;
			uint64_t Value2 = 0;
		};

		using ValuePairStack = CompactArrayStack<ValuePair, 3>;

		void AssertHistoricalValues(const ValuePairStack& stack, const std::array<std::pair<uint64_t, uint64_t>, 3>& expectedValuePairs) {
			auto index = 0u;
			for (const auto& pair : stack) {
				const auto& expectedPair = expectedValuePairs[index];

				// Assert: pair contents
				EXPECT_EQ(expectedPair.first, pair.Value1) << "at " << index;
				EXPECT_EQ(expectedPair.second, pair.Value2) << "at " << index;
				++index;
			}

			// - expected number of values
			EXPECT_EQ(3u, index);
		}
	}

	// endregion

	// region construction

	TEST(TEST_CLASS, CanCreateStack) {
		// Act:
		ValuePairStack stack;

		// Assert:
		EXPECT_EQ(0u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanCopyConstruct_Empty) {
		// Arrange:
		ValuePairStack stack;

		// Act:
		ValuePairStack stackCopy(stack);
		stackCopy.push({ 222, 444 });

		// Assert: the copy is detached from the original
		EXPECT_EQ(0u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });

		EXPECT_EQ(1u, stackCopy.size());
		AssertHistoricalValues(stackCopy, { { std::make_pair(222, 444), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanCopyConstruct_NotEmpty) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 111, 222 });

		// Act:
		ValuePairStack stackCopy(stack);
		stackCopy.push({ 222, 444 });

		// Assert: the copy is detached from the original
		EXPECT_EQ(1u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(111, 222), std::make_pair(0, 0), std::make_pair(0, 0) } });

		EXPECT_EQ(2u, stackCopy.size());
		AssertHistoricalValues(stackCopy, { { std::make_pair(222, 444), std::make_pair(111, 222), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanMoveConstruct_Empty) {
		// Arrange:
		ValuePairStack stack;

		// Act:
		ValuePairStack stackCopy(std::move(stack));
		stackCopy.push({ 222, 444 });

		// Assert: the copy is detached from the original
		EXPECT_EQ(0u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });

		EXPECT_EQ(1u, stackCopy.size());
		AssertHistoricalValues(stackCopy, { { std::make_pair(222, 444), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanMoveConstruct_NotEmpty) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 111, 222 });

		// Act:
		ValuePairStack stackCopy(std::move(stack));
		stackCopy.push({ 222, 444 });

		// Assert: the copy is detached from the original
		EXPECT_EQ(0u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });

		EXPECT_EQ(2u, stackCopy.size());
		AssertHistoricalValues(stackCopy, { { std::make_pair(222, 444), std::make_pair(111, 222), std::make_pair(0, 0) } });
	}

	// endregion

	// region assignment

	TEST(TEST_CLASS, CanAssign_Empty) {
		// Arrange:
		ValuePairStack stack;

		// Act:
		ValuePairStack stackCopy;
		auto& assignResult = stackCopy = stack;
		stackCopy.push({ 222, 444 });

		// Assert: the copy is detached from the original
		EXPECT_EQ(&stackCopy, &assignResult);

		EXPECT_EQ(0u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });

		EXPECT_EQ(1u, stackCopy.size());
		AssertHistoricalValues(stackCopy, { { std::make_pair(222, 444), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanAssign_NotEmpty) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 111, 222 });

		// Act:
		ValuePairStack stackCopy;
		auto& assignResult = stackCopy = stack;
		stackCopy.push({ 222, 444 });

		// Assert: the copy is detached from the original
		EXPECT_EQ(&stackCopy, &assignResult);

		EXPECT_EQ(1u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(111, 222), std::make_pair(0, 0), std::make_pair(0, 0) } });

		EXPECT_EQ(2u, stackCopy.size());
		AssertHistoricalValues(stackCopy, { { std::make_pair(222, 444), std::make_pair(111, 222), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanMoveAssign_Empty) {
		// Arrange:
		ValuePairStack stack;

		// Act:
		ValuePairStack stackCopy;
		auto& assignResult = stackCopy = std::move(stack);
		stackCopy.push({ 222, 444 });

		// Assert: the copy is detached from the original
		EXPECT_EQ(&stackCopy, &assignResult);

		EXPECT_EQ(0u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });

		EXPECT_EQ(1u, stackCopy.size());
		AssertHistoricalValues(stackCopy, { { std::make_pair(222, 444), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanMoveAssign_NotEmpty) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 111, 222 });

		// Act:
		ValuePairStack stackCopy;
		auto& assignResult = stackCopy = std::move(stack);
		stackCopy.push({ 222, 444 });

		// Assert: the copy is detached from the original
		EXPECT_EQ(&stackCopy, &assignResult);

		EXPECT_EQ(0u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });

		EXPECT_EQ(2u, stackCopy.size());
		AssertHistoricalValues(stackCopy, { { std::make_pair(222, 444), std::make_pair(111, 222), std::make_pair(0, 0) } });
	}

	// endregion

	// region peek

	TEST(TEST_CLASS, CannotPeekWhenEmpty) {
		// Arrange:
		ValuePairStack stack;

		// Act + Assert:
		EXPECT_THROW(stack.peek(), catapult_out_of_range);
		EXPECT_THROW(const_cast<const ValuePairStack&>(stack).peek(), catapult_out_of_range);
	}

	TEST(TEST_CLASS, CanPeekWhenNotEmpty) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 111, 222 });
		stack.push({ 333, 444 });

		// Act:
		auto& topMutable = stack.peek();
		auto& topConst = const_cast<const ValuePairStack&>(stack).peek();

		// Assert:
		EXPECT_EQ(&topMutable, &topConst);
		EXPECT_FALSE(std::is_const_v<std::remove_reference_t<decltype(topMutable)>>);
		EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(topConst)>>);

		EXPECT_EQ(333u, topMutable.Value1);
		EXPECT_EQ(444u, topMutable.Value2);

		EXPECT_EQ(333u, topConst.Value1);
		EXPECT_EQ(444u, topConst.Value2);
	}

	// endregion

	// region push

	TEST(TEST_CLASS, CanPushValue) {
		// Act:
		ValuePairStack stack;
		stack.push({ 123, 234 });

		// Assert:
		EXPECT_EQ(1u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(123, 234), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanPushMaxValues) {
		// Act:
		ValuePairStack stack;
		stack.push({ 123, 234 });
		stack.push({ 222, 444 });
		stack.push({ 111, 789 });

		// Assert:
		EXPECT_EQ(3u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(111, 789), std::make_pair(222, 444), std::make_pair(123, 234) } });
	}

	TEST(TEST_CLASS, PushingMoreThanMaxValuesRollsOver) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 123, 234 });
		stack.push({ 222, 444 });
		stack.push({ 111, 789 });

		// Act:
		stack.push({ 332, 888 });

		// Assert:
		EXPECT_EQ(3u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(332, 888), std::make_pair(111, 789), std::make_pair(222, 444) } });
	}

	// endregion

	// region pop

	TEST(TEST_CLASS, CanPopMostRecentValue) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 123, 234 });
		stack.push({ 222, 444 });
		stack.push({ 111, 789 });

		// Act:
		stack.pop();

		// Assert:
		EXPECT_EQ(2u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(222, 444), std::make_pair(123, 234), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanPopAllValues) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 123, 234 });
		stack.push({ 222, 444 });
		stack.push({ 111, 789 });

		// Act:
		stack.pop();
		stack.pop();
		stack.pop();

		// Assert:
		EXPECT_EQ(0u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanPopAllValuesAndThenSetValue) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 123, 234 });
		stack.push({ 222, 444 });
		stack.push({ 111, 789 });

		// Act:
		stack.pop();
		stack.pop();
		stack.pop();
		stack.push({ 333, 321 });

		// Assert:
		EXPECT_EQ(1u, stack.size());
		AssertHistoricalValues(stack, { { std::make_pair(333, 321), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CannotPopMoreValuesThanSet) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 123, 234 });

		// Act: try to pop one more value than set
		stack.pop();
		EXPECT_THROW(stack.pop(), catapult_out_of_range);
	}

	// endregion

	// region iterators

	TEST(TEST_CLASS, CanAdvanceIteratorsPostfixOperator) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 111, 234 });

		// Act:
		auto iter = stack.begin();
		iter++;

		// Assert: only a single value is set, so advancing should always advance to a default value
		EXPECT_EQ(0u, iter->Value1);
		EXPECT_EQ(0u, iter->Value2);
	}

	TEST(TEST_CLASS, CanAdvanceIteratorsPrefixOperator) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 111, 234 });

		// Act:
		auto iter = ++stack.begin();

		// Assert: only a single value is set, so advancing should always advance to a default value
		EXPECT_EQ(0u, iter->Value1);
		EXPECT_EQ(0u, iter->Value2);
	}

	namespace {
		void AssertCanAdvanceIteratorsToEnd(const consumer<ValuePairStack::const_iterator&>& increment) {
			// Arrange:
			ValuePairStack stack;
			stack.push({ 111, 234 });
			stack.push({ 222, 345 });
			stack.push({ 333, 456 });

			// Act:
			auto iter = stack.begin();

			// Assert: first
			EXPECT_NE(stack.end(), iter);
			EXPECT_EQ(333u, iter->Value1);
			EXPECT_EQ(456u, iter->Value2);
			increment(iter);

			// - second
			EXPECT_NE(stack.end(), iter);
			EXPECT_EQ(222u, iter->Value1);
			EXPECT_EQ(345u, iter->Value2);
			increment(iter);

			// - third
			EXPECT_NE(stack.end(), iter);
			EXPECT_EQ(111u, iter->Value1);
			EXPECT_EQ(234u, iter->Value2);
			increment(iter);

			// - end
			EXPECT_EQ(stack.end(), iter);
		}
	}

	TEST(TEST_CLASS, CanAdvanceIteratorsToEndPostfixOperator) {
		AssertCanAdvanceIteratorsToEnd([](auto& iter) { iter++; });
	}

	TEST(TEST_CLASS, CanAdvanceIteratorsToEndPrefixOperator) {
		AssertCanAdvanceIteratorsToEnd([](auto& iter) { ++iter; });
	}

	TEST(TEST_CLASS, CannotAdvanceIteratorsPastEnd) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 111, 234 });

		// Act + Assert:
		EXPECT_THROW(++stack.end(), catapult_out_of_range);
		EXPECT_THROW(stack.end()++, catapult_out_of_range);
	}

	TEST(TEST_CLASS, CannotDereferenceIteratorsAtEnd) {
		// Arrange:
		ValuePairStack stack;
		stack.push({ 111, 234 });

		// Act + Assert:
		EXPECT_THROW(*stack.end(), catapult_out_of_range);
		EXPECT_THROW(stack.end().operator->(), catapult_out_of_range);
	}

	// endregion
}}

