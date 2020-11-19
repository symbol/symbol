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
#include "plugins/txes/lock_shared/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	// region PackedLockInfo

#pragma pack(push, 1)

	/// Packed lock info.
	struct PackedLockInfo {
	public:
		/// Creates a lock info from \a lockInfo.
		explicit PackedLockInfo(const LockInfo& lockInfo)
				: OwnerAddress(lockInfo.OwnerAddress)
				, MosaicId(lockInfo.MosaicId)
				, Amount(lockInfo.Amount)
				, Height(lockInfo.EndHeight)
				, Status(lockInfo.Status)
		{}

	public:
		Address OwnerAddress;
		catapult::MosaicId MosaicId;
		catapult::Amount Amount;
		catapult::Height Height;
		LockStatus Status;
	};

#pragma pack(pop)

	// endregion

	/// Lock info history serializer test suite.
	template<typename TLockInfoTraits>
	class LockInfoHistorySerializerTests {
	private:
		using PackedValueType = typename TLockInfoTraits::PackedValueType;

		static constexpr size_t Lock_Info_Size = sizeof(PackedValueType);

	private:
		static typename TLockInfoTraits::HistoryType CreateHistory(
				const Hash256& hash,
				const std::vector<typename TLockInfoTraits::ValueType>& lockInfos) {
			auto history = typename TLockInfoTraits::HistoryType(hash);
			for (const auto& lockInfo : lockInfos)
				history.push_back(lockInfo);

			return history;
		}

	public:
		// region historical

		static void AssertCanSaveEmptyLockInfoHistory() {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);

			// - prepare a history with zero lock infos
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = typename TLockInfoTraits::HistoryType(hash);

			// Act:
			TLockInfoTraits::SerializerType::Save(history, outputStream);

			// Assert:
			ASSERT_EQ(sizeof(uint64_t) + Hash256::Size, buffer.size());
			EXPECT_EQ(0u, reinterpret_cast<const uint64_t&>(buffer[0]));
			EXPECT_EQ(hash, reinterpret_cast<const Hash256&>(buffer[sizeof(uint64_t)]));
		}

		static void AssertCanSaveSingleLockInfoHistory() {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);

			// - prepare a history with three lock infos
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(3);
			auto hash = TLockInfoTraits::SetEqualIdentifier(lockInfos);
			auto history = CreateHistory(hash, lockInfos);

			// Act:
			TLockInfoTraits::SerializerType::Save(history, outputStream);

			// Assert:
			ASSERT_EQ(sizeof(uint64_t) + Hash256::Size + 3 * Lock_Info_Size, buffer.size());
			ASSERT_EQ(3u, reinterpret_cast<const uint64_t&>(buffer[0]));
			EXPECT_EQ(hash, reinterpret_cast<const Hash256&>(buffer[sizeof(uint64_t)]));

			const auto* pSerializedLockInfos = reinterpret_cast<const PackedValueType*>(&buffer[sizeof(uint64_t) + Hash256::Size]);
			for (auto i = 0u; i < history.historyDepth(); ++i) {
				auto expectedPackedLockInfo = PackedValueType(lockInfos[i]);
				EXPECT_EQ_MEMORY(&expectedPackedLockInfo, &pSerializedLockInfos[i], Lock_Info_Size);
			}
		}

		static void AssertCanRoundtripSingleLockInfoHistory() {
			// Arrange:
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(3);
			auto hash = TLockInfoTraits::SetEqualIdentifier(lockInfos);
			auto originalHistory = CreateHistory(hash, lockInfos);

			// Sanity:
			EXPECT_EQ(3u, originalHistory.historyDepth());

			// Act:
			auto history = test::RunRoundtripBufferTest<typename TLockInfoTraits::SerializerType>(originalHistory);

			// Assert:
			EXPECT_EQ(hash, history.id());
			ASSERT_EQ(3u, history.historyDepth());

			auto originalIter = originalHistory.begin();
			auto iter = history.begin();
			for (auto i = 0u; i < originalHistory.historyDepth(); ++i)
				TLockInfoTraits::AssertEqual(*originalIter++, *iter++);
		}

		// endregion

		// region non historical

		static void AssertCannotSaveEmptyLockInfoHistoryNonHistorical() {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);

			// - prepare a history with zero lock infos
			auto hash = test::GenerateRandomByteArray<Hash256>();
			auto history = typename TLockInfoTraits::HistoryType(hash);

			// Act + Assert:
			EXPECT_THROW(TLockInfoTraits::NonHistoricalSerializerType::Save(history, outputStream), catapult_runtime_error);
		}

		static void AssertCanSaveSingleLockInfoHistoryNonHistorical() {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);

			// - prepare a history with three lock infos
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(3);
			auto hash = TLockInfoTraits::SetEqualIdentifier(lockInfos);
			auto history = CreateHistory(hash, lockInfos);

			// Act:
			TLockInfoTraits::NonHistoricalSerializerType::Save(history, outputStream);

			// Assert: only most recent lock info is stored
			ASSERT_EQ(Lock_Info_Size, buffer.size());

			auto expectedPackedLockInfo = PackedValueType(lockInfos.back());
			EXPECT_EQ_MEMORY(&expectedPackedLockInfo, buffer.data(), buffer.size());
		}

		static void AssertCanRoundtripSingleLockInfoHistoryNonHistorical() {
			// Arrange:
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(3);
			auto hash = TLockInfoTraits::SetEqualIdentifier(lockInfos);
			auto originalHistory = CreateHistory(hash, lockInfos);

			// Sanity:
			EXPECT_EQ(3u, originalHistory.historyDepth());

			// Act:
			auto history = test::RunRoundtripBufferTest<typename TLockInfoTraits::NonHistoricalSerializerType>(originalHistory);

			// Assert:
			EXPECT_EQ(hash, history.id());
			EXPECT_EQ(1u, history.historyDepth());

			TLockInfoTraits::AssertEqual(originalHistory.back(), history.back());
		}

		// endregion
	};
}}

#define MAKE_LOCK_INFO_HISTORY_SERIALIZER_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockInfoHistorySerializerTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_INFO_HISTORY_SERIALIZER_TESTS(TRAITS_NAME) \
	MAKE_LOCK_INFO_HISTORY_SERIALIZER_TEST(TRAITS_NAME, CanSaveEmptyLockInfoHistory) \
	MAKE_LOCK_INFO_HISTORY_SERIALIZER_TEST(TRAITS_NAME, CanSaveSingleLockInfoHistory) \
	MAKE_LOCK_INFO_HISTORY_SERIALIZER_TEST(TRAITS_NAME, CanRoundtripSingleLockInfoHistory) \
	\
	MAKE_LOCK_INFO_HISTORY_SERIALIZER_TEST(TRAITS_NAME, CannotSaveEmptyLockInfoHistoryNonHistorical) \
	MAKE_LOCK_INFO_HISTORY_SERIALIZER_TEST(TRAITS_NAME, CanSaveSingleLockInfoHistoryNonHistorical) \
	MAKE_LOCK_INFO_HISTORY_SERIALIZER_TEST(TRAITS_NAME, CanRoundtripSingleLockInfoHistoryNonHistorical)
