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

namespace catapult { namespace state {

	/// Lock info history test suite.
	template<typename TLockInfoHistory, typename TTraits>
	class LockInfoHistoryTests {
	public:
		// region constructor

		static void AssertCanCreateEmptyHistoryWithIdentifier() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();

			// Act:
			auto history = TLockInfoHistory(hash);

			// Assert:
			AssertEmpty(history, hash);
		}

		// endregion

		// region push_back

		static void AssertCanPushLockInfoWithMatchingIdentifier() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = TLockInfoHistory(hash);

			// Act:
			history.push_back(TTraits::CreateLockInfo(Height(11), hash));

			// Assert:
			EXPECT_EQ(hash, history.id());
			EXPECT_FALSE(history.empty());
			EXPECT_EQ(1u, history.historyDepth());
			EXPECT_EQ(std::vector<Height>({ Height(11) }), GetLifetimeEndHeights(history));
		}

		static void AssertCanPushMultipleLockInfosWithMatchingIdentifier() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = TLockInfoHistory(hash);

			// Act:
			history.push_back(TTraits::CreateLockInfo(Height(11), hash));
			history.push_back(TTraits::CreateLockInfo(Height(99), hash));
			history.push_back(TTraits::CreateLockInfo(Height(67), hash));

			// Assert:
			EXPECT_EQ(hash, history.id());
			EXPECT_FALSE(history.empty());
			EXPECT_EQ(3u, history.historyDepth());
			EXPECT_EQ(std::vector<Height>({ Height(11), Height(99), Height(67) }), GetLifetimeEndHeights(history));
		}

		static void AssertCannotPushLockInfoWithDifferentIdentifier() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = TLockInfoHistory(hash);
			history.push_back(TTraits::CreateLockInfo(Height(11), hash));

			// Act + Assert:
			auto hash2 = test::GenerateRandomByteArray<Hash256>();
			EXPECT_THROW(history.push_back(TTraits::CreateLockInfo(Height(99), hash2)), catapult_invalid_argument);

			// Sanity:
			EXPECT_EQ(hash, history.id());
			EXPECT_FALSE(history.empty());
			EXPECT_EQ(1u, history.historyDepth());
			EXPECT_EQ(std::vector<Height>({ Height(11) }), GetLifetimeEndHeights(history));
		}

		// endregion

		// region pop_back

		static void AssertCanPopLockInfoWhenNotEmpty() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = CreateDefaultHistory(hash);

			// Act:
			history.pop_back();

			// Assert:
			EXPECT_EQ(hash, history.id());
			EXPECT_FALSE(history.empty());
			EXPECT_EQ(2u, history.historyDepth());
			EXPECT_EQ(std::vector<Height>({ Height(11), Height(99) }), GetLifetimeEndHeights(history));
		}

		static void AssertCanPopLockInfoUntilEmpty() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = CreateDefaultHistory(hash);

			// Act:
			history.pop_back();
			history.pop_back();
			history.pop_back();

			// Assert:
			AssertEmpty(history, hash);
		}

		static void AssertCannotPopLockInfoWhenEmpty() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = CreateDefaultHistory(hash);

			history.pop_back();
			history.pop_back();
			history.pop_back();

			// Act + Assert:
			EXPECT_THROW(history.pop_back(), catapult_invalid_argument);
		}

		// endregion

		// region back

		static void AssertBackReturnsMostRecentLockInfo() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = CreateDefaultHistory(hash);

			// Act:
			auto& lockInfo = history.back();
			auto& constLockInfo = const_cast<const TLockInfoHistory&>(history).back();

			// Assert:
			EXPECT_EQ(hash, history.id());

			EXPECT_FALSE(std::is_const_v<std::remove_reference_t<decltype(lockInfo)>>);
			EXPECT_EQ(&constLockInfo, &lockInfo);

			EXPECT_EQ(Height(67), lockInfo.EndHeight);
			EXPECT_EQ(Height(67), constLockInfo.EndHeight);
		}

		// endregion

		// region isActive

		static void AssertIsActiveReturnsFalseWhenHistoryIsEmpty() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = TLockInfoHistory(hash);

			// Sanity:
			EXPECT_TRUE(history.empty());

			// Act + Assert:
			EXPECT_FALSE(history.isActive(Height(250)));
		}

		static void AssertIsActiveReturnsCorrectValueWhenLastLockIsUsed() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = CreateDefaultHistory(hash, LockStatus::Used);

			// Act + Assert: only last lock is relevant to result
			EXPECT_FALSE(history.isActive(Height(60)));
			EXPECT_FALSE(history.isActive(Height(66)));
			EXPECT_FALSE(history.isActive(Height(67)));
			EXPECT_FALSE(history.isActive(Height(80)));
		}

		static void AssertIsActiveReturnsCorrectValueWhenLastLockIsNotUsed() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = CreateDefaultHistory(hash, LockStatus::Unused);

			// Act + Assert: only last lock is relevant to result
			EXPECT_TRUE(history.isActive(Height(60)));
			EXPECT_TRUE(history.isActive(Height(66)));
			EXPECT_FALSE(history.isActive(Height(67)));
			EXPECT_FALSE(history.isActive(Height(80)));
		}

		// endregion

		// region prune

		static void AssertPruneIsNoOpWhenNoLockInfosAreExpired() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = TLockInfoHistory(hash);
			history.push_back(TTraits::CreateLockInfo(Height(11), hash));
			history.push_back(TTraits::CreateLockInfo(Height(67), hash));
			history.push_back(TTraits::CreateLockInfo(Height(99), hash));

			// Act:
			history.prune(Height(10));

			// Assert:
			EXPECT_EQ(hash, history.id());
			EXPECT_FALSE(history.empty());
			EXPECT_EQ(3u, history.historyDepth());
			EXPECT_EQ(std::vector<Height>({ Height(11), Height(67), Height(99) }), GetLifetimeEndHeights(history));
		}

		static void AssertPruneCanPartiallyPruneHistoryWhenSomeLockInfosAreExpired() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = TLockInfoHistory(hash);
			history.push_back(TTraits::CreateLockInfo(Height(11), hash));
			history.push_back(TTraits::CreateLockInfo(Height(67), hash));
			history.push_back(TTraits::CreateLockInfo(Height(99), hash));

			// Act:
			history.prune(Height(67));

			// Assert:
			EXPECT_EQ(hash, history.id());
			EXPECT_FALSE(history.empty());
			EXPECT_EQ(1u, history.historyDepth());
			EXPECT_EQ(std::vector<Height>({ Height(99) }), GetLifetimeEndHeights(history));
		}

		static void AssertPruneCanPartiallyPruneHistoryWhenAllLockInfosAreExpired() {
			// Arrange:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = TLockInfoHistory(hash);
			history.push_back(TTraits::CreateLockInfo(Height(11), hash));
			history.push_back(TTraits::CreateLockInfo(Height(67), hash));
			history.push_back(TTraits::CreateLockInfo(Height(99), hash));

			// Act:
			history.prune(Height(120));

			// Assert:
			EXPECT_EQ(hash, history.id());
			EXPECT_TRUE(history.empty());
			EXPECT_EQ(0u, history.historyDepth());
			EXPECT_EQ(std::vector<Height>(), GetLifetimeEndHeights(history));
		}

		// endregion

	private:
		// region test utils

		static TLockInfoHistory CreateDefaultHistory(const Hash256& hash, LockStatus status = LockStatus::Unused) {
			auto history = TLockInfoHistory(hash);
			for (auto height : std::initializer_list<Height>{ Height(11), Height(99), Height(67) }) {
				history.push_back(TTraits::CreateLockInfo(height, hash));
				history.back().Status = status;
			}

			return history;
		}

		static std::vector<Height> GetLifetimeEndHeights(const TLockInfoHistory& history) {
			// check the end heights as a way to validate iteration
			std::vector<Height> heights;
			for (const auto& lockInfo : history)
				heights.push_back(lockInfo.EndHeight);

			return heights;
		}

		// endregion

		// region test asserts

		static void AssertEmpty(const TLockInfoHistory& history, const Hash256& expectedId) {
			EXPECT_EQ(expectedId, history.id());
			EXPECT_TRUE(history.empty());
			EXPECT_EQ(0u, history.historyDepth());
			EXPECT_EQ(std::vector<Height>(), GetLifetimeEndHeights(history));
		}

		// endregion
	};
}}

#define MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockInfoHistoryTests<LOCK_INFO_HISTORY_TYPE, LOCK_INFO_HISTORY_TYPE##Traits>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_INFO_HISTORY_TESTS(LOCK_INFO_HISTORY_TYPE) \
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, CanCreateEmptyHistoryWithIdentifier) \
	\
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, CanPushLockInfoWithMatchingIdentifier) \
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, CanPushMultipleLockInfosWithMatchingIdentifier) \
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, CannotPushLockInfoWithDifferentIdentifier) \
	\
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, CanPopLockInfoWhenNotEmpty) \
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, CanPopLockInfoUntilEmpty) \
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, CannotPopLockInfoWhenEmpty) \
	\
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, BackReturnsMostRecentLockInfo) \
	\
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, IsActiveReturnsFalseWhenHistoryIsEmpty) \
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, IsActiveReturnsCorrectValueWhenLastLockIsUsed) \
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, IsActiveReturnsCorrectValueWhenLastLockIsNotUsed) \
	\
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, PruneIsNoOpWhenNoLockInfosAreExpired) \
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, PruneCanPartiallyPruneHistoryWhenSomeLockInfosAreExpired) \
	MAKE_LOCK_INFO_HISTORY_TEST(LOCK_INFO_HISTORY_TYPE, PruneCanPartiallyPruneHistoryWhenAllLockInfosAreExpired)
