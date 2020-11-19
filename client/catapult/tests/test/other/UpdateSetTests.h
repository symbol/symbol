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
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Update set test suite.
	template<typename TTraits>
	class UpdateSetTests {
	public:
		// region zero operations

		static void AssertOriginalSetIsUnchangedWhenDeltasAreEmpty() {
			// Arrange:
			typename TTraits::TestContext context;

			// Act:
			TTraits::CommitPolicy::Update(context.Set, context.deltas());

			// Assert:
			EXPECT_EQ(3u, context.Set.size());
			EXPECT_TRUE(TTraits::Contains(context.Set, "aaa", 1));
			EXPECT_TRUE(TTraits::Contains(context.Set, "ccc", 3));
			EXPECT_TRUE(TTraits::Contains(context.Set, "ddd", 2));
		}

		// endregion

		// region single operations - simple

		static void AssertDeltaAdditionsCanBeAppliedToOriginalSet() {
			// Arrange:
			typename TTraits::TestContext context;
			TTraits::AddElement(context.Added, "eee", 7);
			TTraits::AddElement(context.Added, "bbb", 5);

			// Act:
			TTraits::CommitPolicy::Update(context.Set, context.deltas());

			// Assert:
			EXPECT_EQ(5u, context.Set.size());
			EXPECT_TRUE(TTraits::Contains(context.Set, "aaa", 1));
			EXPECT_TRUE(TTraits::Contains(context.Set, "ccc", 3));
			EXPECT_TRUE(TTraits::Contains(context.Set, "ddd", 2));
			EXPECT_TRUE(TTraits::Contains(context.Set, "eee", 7));
			EXPECT_TRUE(TTraits::Contains(context.Set, "bbb", 5));
		}

		static void AssertDeltaRemovalsCanBeAppliedToOriginalSet() {
			// Arrange:
			typename TTraits::TestContext context;
			TTraits::AddElement(context.Removed, "aaa", 1);
			TTraits::AddElement(context.Removed, "ccc", 3);

			// Act:
			TTraits::CommitPolicy::Update(context.Set, context.deltas());

			// Assert:
			EXPECT_EQ(1u, context.Set.size());
			EXPECT_TRUE(TTraits::Contains(context.Set, "ddd", 2));

			EXPECT_FALSE(TTraits::Contains(context.Set, "aaa", 1));
			EXPECT_FALSE(TTraits::Contains(context.Set, "ccc", 3));
		}

		static void AssertDeltaCopiesCanBeAppliedToOriginalSet() {
			// Arrange:
			typename TTraits::TestContext context;
			TTraits::AddElement(context.Copied, "aaa", 1, 10);
			TTraits::AddElement(context.Copied, "ccc", 3, 11);

			// Act:
			TTraits::CommitPolicy::Update(context.Set, context.deltas());

			// Assert:
			EXPECT_EQ(3u, context.Set.size());
			EXPECT_TRUE(TTraits::Contains(context.Set, "aaa", 1, 10));
			EXPECT_TRUE(TTraits::Contains(context.Set, "ccc", 3, 11));
			EXPECT_TRUE(TTraits::Contains(context.Set, "ddd", 2));

			// Sanity:
			EXPECT_FALSE(TTraits::Contains(context.Set, "aaa", 1));
			EXPECT_FALSE(TTraits::Contains(context.Set, "ccc", 3));
		}

		// endregion

		// region single operations - elements unrelated to base set

		static void AssertDeltaRedundantAdditionsAreIgnored() {
			// Arrange:
			typename TTraits::TestContext context;
			TTraits::AddElement(context.Added, "aaa", 1);
			TTraits::AddElement(context.Added, "eee", 7);
			TTraits::AddElement(context.Added, "bbb", 5);
			TTraits::AddElement(context.Added, "ddd", 2);

			// Act:
			TTraits::CommitPolicy::Update(context.Set, context.deltas());

			// Assert:
			EXPECT_EQ(5u, context.Set.size());
			EXPECT_TRUE(TTraits::Contains(context.Set, "aaa", 1));
			EXPECT_TRUE(TTraits::Contains(context.Set, "ccc", 3));
			EXPECT_TRUE(TTraits::Contains(context.Set, "ddd", 2));
			EXPECT_TRUE(TTraits::Contains(context.Set, "eee", 7));
			EXPECT_TRUE(TTraits::Contains(context.Set, "bbb", 5));
		}

		static void AssertDeltaUnknownRemovalsAreIgnored() {
			// Arrange:
			typename TTraits::TestContext context;
			TTraits::AddElement(context.Removed, "eee", 7);
			TTraits::AddElement(context.Removed, "aaa", 1);
			TTraits::AddElement(context.Removed, "ccc", 3);
			TTraits::AddElement(context.Removed, "bbb", 5);

			// Act:
			TTraits::CommitPolicy::Update(context.Set, context.deltas());

			// Assert:
			EXPECT_EQ(1u, context.Set.size());
			EXPECT_TRUE(TTraits::Contains(context.Set, "ddd", 2));

			EXPECT_FALSE(TTraits::Contains(context.Set, "aaa", 1));
			EXPECT_FALSE(TTraits::Contains(context.Set, "ccc", 3));
		}

		static void AssertDeltaUnknownCopiesAreNotSupported() {
			// Arrange:
			typename TTraits::TestContext context;
			TTraits::AddElement(context.Copied, "eee", 7, 10);

			// Act:
			EXPECT_THROW(TTraits::CommitPolicy::Update(context.Set, context.deltas()), catapult_invalid_argument);
		}

		// endregion

		// region multiple operations

		static void AssertAllDeltaChangesAreAppliedToOriginalSet() {
			// Arrange:
			typename TTraits::TestContext context;
			TTraits::AddElement(context.Added, "bbb", 5);
			TTraits::AddElement(context.Removed, "ccc", 3);
			TTraits::AddElement(context.Copied, "aaa", 1, 10);

			// Act:
			TTraits::CommitPolicy::Update(context.Set, context.deltas());

			// Assert:
			EXPECT_EQ(3u, context.Set.size());
			EXPECT_TRUE(TTraits::Contains(context.Set, "aaa", 1, 10)); // copied
			EXPECT_TRUE(TTraits::Contains(context.Set, "ddd", 2)); // unchanged
			EXPECT_TRUE(TTraits::Contains(context.Set, "bbb", 5)); // added

			EXPECT_FALSE(TTraits::Contains(context.Set, "ccc", 3)); // removed
		}

		static void AssertAllDeltaChangesAreAppliedToOriginalSetInDeterministicOrder() {
			// Arrange: added < copied < removed
			typename TTraits::TestContext context;
			TTraits::AddElement(context.Added, "bbb", 5);
			TTraits::AddElement(context.Copied, "bbb", 5, 10);
			TTraits::AddElement(context.Removed, "bbb", 5);

			TTraits::AddElement(context.Copied, "ddd", 2, 12);
			TTraits::AddElement(context.Removed, "ddd", 2);

			TTraits::AddElement(context.Added, "fff", 6);
			TTraits::AddElement(context.Copied, "fff", 6, 11);

			TTraits::AddElement(context.Added, "zzz", 7);

			// Act:
			TTraits::CommitPolicy::Update(context.Set, context.deltas());

			// Assert:
			EXPECT_EQ(4u, context.Set.size());
			EXPECT_TRUE(TTraits::Contains(context.Set, "aaa", 1)); // unchanged
			EXPECT_TRUE(TTraits::Contains(context.Set, "ccc", 3)); // unchanged
			EXPECT_TRUE(TTraits::Contains(context.Set, "fff", 6, 11)); // added + copied
			EXPECT_TRUE(TTraits::Contains(context.Set, "zzz", 7)); // added

			EXPECT_FALSE(TTraits::Contains(context.Set, "bbb", 5)); // added + copied + removed
			EXPECT_FALSE(TTraits::Contains(context.Set, "bbb", 5, 10)); // added + copied + removed
			EXPECT_FALSE(TTraits::Contains(context.Set, "ddd", 2)); // copied + removed
			EXPECT_FALSE(TTraits::Contains(context.Set, "ddd", 2, 12)); // copied + removed
			EXPECT_FALSE(TTraits::Contains(context.Set, "fff", 6)); // added + copied
		}

		// endregion
	};

#define MAKE_UPDATE_SET_TEST(TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { \
		test::UpdateSetTests<TRAITS>::Assert##TEST_NAME(); \
	}

#define DEFINE_UPDATE_SET_TESTS(TRAITS) \
	MAKE_UPDATE_SET_TEST(TRAITS, OriginalSetIsUnchangedWhenDeltasAreEmpty) \
	MAKE_UPDATE_SET_TEST(TRAITS, DeltaAdditionsCanBeAppliedToOriginalSet) \
	MAKE_UPDATE_SET_TEST(TRAITS, DeltaRemovalsCanBeAppliedToOriginalSet) \
	MAKE_UPDATE_SET_TEST(TRAITS, DeltaCopiesCanBeAppliedToOriginalSet) \
	MAKE_UPDATE_SET_TEST(TRAITS, AllDeltaChangesAreAppliedToOriginalSet) \
	MAKE_UPDATE_SET_TEST(TRAITS, AllDeltaChangesAreAppliedToOriginalSetInDeterministicOrder)

#define DEFINE_MEMORY_ONLY_UPDATE_SET_TESTS(TRAITS) \
	MAKE_UPDATE_SET_TEST(TRAITS, DeltaRedundantAdditionsAreIgnored) \
	MAKE_UPDATE_SET_TEST(TRAITS, DeltaUnknownRemovalsAreIgnored) \
	MAKE_UPDATE_SET_TEST(TRAITS, DeltaUnknownCopiesAreNotSupported)
}}
