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

	/// Lock info serializer test suite.
	template<typename TLockInfoTraits>
	class LockInfoSerializerTests {
	private:
		static void AssertBuffer(const typename TLockInfoTraits::ValueType& expectedValue, const std::vector<uint8_t>& buffer) {
			std::vector<typename TLockInfoTraits::PackedValueType> packedValues;
			packedValues.emplace_back(expectedValue);

			ASSERT_EQ(sizeof(typename TLockInfoTraits::PackedValueType) * packedValues.size(), buffer.size());
			EXPECT_EQ_MEMORY(packedValues.data(), buffer.data(), buffer.size());
		}

	public:
		static void AssertCanSaveSingleLockInfo() {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);

			auto lockInfo = test::CreateLockInfos<TLockInfoTraits>(1)[0];

			// Act:
			TLockInfoTraits::SerializerType::Save(lockInfo, outputStream);

			// Assert:
			AssertBuffer(lockInfo, buffer);
		}

		static void AssertCanRoundtripSingleLockInfo() {
			// Arrange:
			auto originalLockInfo = test::CreateLockInfos<TLockInfoTraits>(1)[0];

			// Act:
			auto result = test::RunRoundtripBufferTest<typename TLockInfoTraits::SerializerType>(originalLockInfo);

			// Assert:
			TLockInfoTraits::AssertEqual(originalLockInfo, result);
		}
	};
}}

#define MAKE_LOCK_INFO_SERIALIZER_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockInfoSerializerTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_INFO_SERIALIZER_TESTS(TRAITS_NAME) \
	MAKE_LOCK_INFO_SERIALIZER_TEST(TRAITS_NAME, CanSaveSingleLockInfo) \
	MAKE_LOCK_INFO_SERIALIZER_TEST(TRAITS_NAME, CanRoundtripSingleLockInfo)
