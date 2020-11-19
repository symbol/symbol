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
#include "BaseSetCommonTests.h"

namespace catapult { namespace test {

	/// All tests that apply to BaseSet but not BaseSetDelta.
	template<typename TTraits>
	class BaseSetTests {
	public:
		// region insert / remove

		static void AssertInsertDoesNotChangeOriginalBaseSet() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);
			auto pDelta = pBaseSet->rebase();
			auto expectedElements = TTraits::CreateElements(3);

			// Act:
			auto result = pDelta->emplace("MyElement", static_cast<unsigned int>(123));

			// Assert:
			EXPECT_EQ(deltaset::InsertResult::Inserted, result);

			EXPECT_EQ(4u, pDelta->size());
			TTraits::AssertContents(*pBaseSet, expectedElements);
			AssertDeltaSizes(*pBaseSet, *pDelta, 3, 1, 0, 0);
		}

		static void AssertRemoveDoesNotChangeOriginalBaseSet() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);
			auto pDelta = pBaseSet->rebase();
			auto expectedElements = TTraits::CreateElements(3);

			// Act:
			auto result = pDelta->remove(TTraits::CreateKey("TestElement", 2));

			// Assert:
			EXPECT_EQ(deltaset::RemoveResult::Removed, result);

			EXPECT_EQ(2u, pDelta->size());
			TTraits::AssertContents(*pBaseSet, expectedElements);
			AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 1, 0);
		}

		// endregion

		// region rebase / rebaseDetached

		static void AssertRebaseCreatesDeltaAroundSuppliedElements() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);
			auto expectedElements = TTraits::CreateElements(3);

			// Act:
			auto pDelta = pBaseSet->rebase();

			// Assert:
			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 0, 0);
		}

		static void AssertRebaseAllowsOnlyOneAttachedDeltaAtATime() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);
			auto expectedElements = TTraits::CreateElements(3);
			{
				auto pDelta = pBaseSet->rebase();

				// Act + Assert:
				EXPECT_THROW(pBaseSet->rebase(), catapult_runtime_error);
			}

			// Act: delta went out of scope, another delta is allowed
			auto pDelta = pBaseSet->rebase();
			TTraits::AssertContents(*pDelta, expectedElements);
			AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 0, 0);
		}

		static void AssertRebaseDetachedCreatesDeltaAroundSuppliedElements() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);
			auto expectedElements = TTraits::CreateElements(3);

			// Act:
			auto pDetachedDelta = pBaseSet->rebaseDetached();

			// Assert:
			TTraits::AssertContents(*pDetachedDelta, expectedElements);
			AssertDeltaSizes(*pBaseSet, *pDetachedDelta, 3, 0, 0, 0);
		}

		static void AssertRebaseDetachedAllowsManyDeltas() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);
			auto expectedElements = TTraits::CreateElements(3);
			std::vector<decltype(pBaseSet->rebaseDetached())> deltas;

			// Act:
			for (auto i = 0u; i < 10; ++i)
				deltas.push_back(pBaseSet->rebaseDetached());

			// Assert:
			for (const auto& pDetachedDelta : deltas) {
				TTraits::AssertContents(*pDetachedDelta, expectedElements);
				AssertDeltaSizes(*pBaseSet, *pDetachedDelta, 3, 0, 0, 0);
			}
		}

		// endregion

		// region commit

		static void AssertCannotCommitWhenThereAreNoPendingAttachedDeltas() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);

			// Act + Assert:
			EXPECT_THROW(TTraits::Commit(*pBaseSet), catapult_runtime_error);
		}

		static void AssertCommitThrowsWhenOnlyDetachedDeltasAreOutstanding() {
			// Arrange:
			auto pBaseSet = TTraits::Create();
			auto pDetachedDelta = pBaseSet->rebaseDetached();

			// Act + Assert:
			EXPECT_THROW(TTraits::Commit(*pBaseSet), catapult_runtime_error);
		}

		static void AssertCommitCommitsToOriginalElements() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);
			auto pDelta = pBaseSet->rebase();
			pDelta->emplace("MyTestElement", static_cast<unsigned int>(123));
			pDelta->remove(TTraits::CreateKey("TestElement", 0));
			pDelta->remove(TTraits::CreateKey("TestElement", 2));
			auto expectedElements = typename TTraits::ElementVector{
				TTraits::CreateElement("TestElement", 1),
				TTraits::CreateElement("MyTestElement", 123)
			};

			// Act:
			TTraits::Commit(*pBaseSet);

			// Assert:
			TTraits::AssertContents(*pBaseSet, expectedElements);
			AssertDeltaSizes(*pBaseSet, *pDelta, 2, 0, 0, 0);
		}

		static void AssertCommitReflectsChangesOnOriginalElements() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);
			auto pDelta = pBaseSet->rebase();
			auto pElement = pDelta->find(TTraits::CreateKey("TestElement", 1)).get();

			// Act:
			pElement->Dummy = 123;
			TTraits::Commit(*pBaseSet);
			auto pElementAfterCommit = pBaseSet->find(TTraits::CreateKey("TestElement", 1)).get();

			// Assert:
			EXPECT_EQ(123u, pElementAfterCommit->Dummy);
			AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 0, 0);
		}

		static void AssertCommitIsIdempotent() {
			// Arrange:
			auto pBaseSet = TTraits::CreateWithElements(3);
			auto pDelta = pBaseSet->rebase();
			pDelta->emplace("MyTestElement", static_cast<unsigned int>(123));
			pDelta->remove(TTraits::CreateKey("TestElement", 0));
			pDelta->remove(TTraits::CreateKey("TestElement", 2));
			auto expectedElements = typename TTraits::ElementVector{
				TTraits::CreateElement("TestElement", 1),
				TTraits::CreateElement("MyTestElement", 123)
			};

			// Act:
			for (auto i = 0u; i < 5; ++i)
				TTraits::Commit(*pBaseSet);

			// Assert:
			TTraits::AssertContents(*pBaseSet, expectedElements);
			AssertDeltaSizes(*pBaseSet, *pDelta, 2, 0, 0, 0);
		}

		// endregion
	};

#define MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BaseSetTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_BASE_SET_TESTS(TEST_CLASS, TRAITS) \
	DEFINE_COMMON_BASE_SET_TESTS(TEST_CLASS, TRAITS) \
	DEFINE_BASE_SET_ITERATION_TESTS(TEST_CLASS, TRAITS) \
	\
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, InsertDoesNotChangeOriginalBaseSet) \
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, RemoveDoesNotChangeOriginalBaseSet) \
	\
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, RebaseCreatesDeltaAroundSuppliedElements) \
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, RebaseAllowsOnlyOneAttachedDeltaAtATime) \
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, RebaseDetachedCreatesDeltaAroundSuppliedElements) \
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, RebaseDetachedAllowsManyDeltas) \
	\
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, CannotCommitWhenThereAreNoPendingAttachedDeltas) \
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, CommitThrowsWhenOnlyDetachedDeltasAreOutstanding) \
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, CommitCommitsToOriginalElements) \
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, CommitIsIdempotent)

#define DEFINE_MUTABLE_BASE_SET_TESTS(TEST_CLASS, TRAITS) \
	DEFINE_BASE_SET_TESTS(TEST_CLASS, TRAITS) \
	\
	MAKE_BASE_SET_TEST(TEST_CLASS, TRAITS, CommitReflectsChangesOnOriginalElements)

#define DEFINE_IMMUTABLE_BASE_SET_TESTS DEFINE_BASE_SET_TESTS

#define DEFINE_MUTABLE_BASE_SET_TESTS_FOR(NAME) \
	DEFINE_MUTABLE_BASE_SET_TESTS(Base##NAME##Tests, test::BaseTraits<deltaset::NAME##Traits>)

#define DEFINE_IMMUTABLE_BASE_SET_TESTS_FOR(NAME) \
	DEFINE_IMMUTABLE_BASE_SET_TESTS(Base##NAME##Tests, test::BaseTraits<deltaset::NAME##Traits>)
}}
