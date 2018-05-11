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

#include "src/mappers/LockInfoMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/LockInfoTestTraits.h"
#include "tests/test/LockMapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS LockInfoMapperTests

	namespace {
		void AssertEqualLockMetadata(const bsoncxx::document::view& dbMetadata) {
			EXPECT_EQ(0u, test::GetFieldCount(dbMetadata));
		}
	}

	// region ToDbModel

#define LOCK_TYPE_BASED_TEST(TEST_NAME) \
	template<typename TLockInfoTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Hash) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::MongoHashLockInfoTestTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Secret) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::MongoSecretLockInfoTestTraits>(); } \
	template<typename TLockInfoTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	LOCK_TYPE_BASED_TEST(CanMapLockInfo_ModelToDbModel) {
		// Arrange:
		auto lockInfo = TLockInfoTraits::CreateLockInfo(Height(123));
		auto address = test::GenerateRandomData<Address_Decoded_Size>();
		lockInfo.Status = model::LockStatus::Used;

		// Act:
		auto document = ToDbModel(lockInfo, address);
		auto view = document.view();

		// Assert:
		EXPECT_EQ(2u, test::GetFieldCount(view));

		auto metaView = view["meta"].get_document().view();
		AssertEqualLockMetadata(metaView);

		auto lockInfoView = view["lock"].get_document().view();
		EXPECT_EQ(6u + TLockInfoTraits::Num_Additional_Fields, test::GetFieldCount(lockInfoView));
		test::AssertEqualLockInfoData(lockInfo, address, lockInfoView);
	}

	// endregion

	// region ToLockInfo

	LOCK_TYPE_BASED_TEST(CanMapLockInfo_DbModelToModel) {
		// Arrange:
		auto originalLockInfo = TLockInfoTraits::CreateLockInfo(Height(123));
		auto address = test::GenerateRandomData<Address_Decoded_Size>();
		originalLockInfo.Status = model::LockStatus::Used;
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
		test::AssertEqualLockInfoData(lockInfo, address, lockInfoView);
	}

	// endregion
}}}
