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
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Lock info mapper test suite.
	template<typename TLockInfoTraits>
	class LockInfoMapperTests {
	private:
		static void AssertEqualLockMetadata(const bsoncxx::document::view& dbMetadata) {
			EXPECT_EQ(0u, test::GetFieldCount(dbMetadata));
		}

	public:
		static void AssertCanMapLockInfo_ModelToDbModel() {
			// Arrange:
			auto lockInfo = TLockInfoTraits::CreateLockInfo(Height(123));
			auto address = test::GenerateRandomData<Address_Decoded_Size>();
			lockInfo.Status = state::LockStatus::Used;

			// Act:
			auto document = ToDbModel(lockInfo, address);
			auto view = document.view();

			// Assert:
			EXPECT_EQ(2u, test::GetFieldCount(view));

			auto metaView = view["meta"].get_document().view();
			AssertEqualLockMetadata(metaView);

			auto lockInfoView = view["lock"].get_document().view();
			EXPECT_EQ(6u + TLockInfoTraits::Num_Additional_Fields, test::GetFieldCount(lockInfoView));
			TLockInfoTraits::AssertEqualLockInfoData(lockInfo, address, lockInfoView);
		}

		static void AssertCanMapLockInfo_DbModelToModel() {
			// Arrange:
			auto originalLockInfo = TLockInfoTraits::CreateLockInfo(Height(123));
			auto address = test::GenerateRandomData<Address_Decoded_Size>();
			originalLockInfo.Status = state::LockStatus::Used;
			auto dbLockInfo = ToDbModel(originalLockInfo, address);

			// Act:
			typename TLockInfoTraits::ValueType lockInfo;
			ToLockInfo(dbLockInfo, lockInfo);

			// Assert:
			auto view = dbLockInfo.view();
			EXPECT_EQ(2u, test::GetFieldCount(view));

			auto metaView = view["meta"].get_document().view();
			AssertEqualLockMetadata(metaView);

			auto lockInfoView = view["lock"].get_document().view();
			EXPECT_EQ(6u + TLockInfoTraits::Num_Additional_Fields, test::GetFieldCount(lockInfoView));
			TLockInfoTraits::AssertEqualLockInfoData(lockInfo, address, lockInfoView);
		}
	};
}}}

#define MAKE_LOCK_INFO_MAPPER_TEST(TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockInfoMapperTests<TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_INFO_MAPPER_TESTS(TRAITS) \
	MAKE_LOCK_INFO_MAPPER_TEST(TRAITS, CanMapLockInfo_ModelToDbModel) \
	MAKE_LOCK_INFO_MAPPER_TEST(TRAITS, CanMapLockInfo_DbModelToModel)
