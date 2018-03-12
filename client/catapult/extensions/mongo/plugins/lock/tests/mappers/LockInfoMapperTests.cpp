#include "src/mappers/LockInfoMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/LockInfoTestTraits.h"
#include "tests/test/LockMapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS LockInfoMapperTests

	namespace {
		constexpr auto Document_Name = "lock";
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
		auto documentView = document.view();

		// Assert:
		EXPECT_EQ(1u, test::GetFieldCount(documentView));

		auto lockInfoView = documentView[Document_Name].get_document().view();
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
		EXPECT_EQ(1u, test::GetFieldCount(view));

		auto lockInfoView = view[Document_Name].get_document().view();
		EXPECT_EQ(6u + TLockInfoTraits::Num_Additional_Fields, test::GetFieldCount(lockInfoView));
		test::AssertEqualLockInfoData(lockInfo, address, lockInfoView);
	}

	// endregion
}}}
