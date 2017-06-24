#include "src/mappers/MultisigEntryMapper.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/mongo/multisig/tests/test/MapperTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/TestHarness.h"

#define TEST_CLASS MultisigEntryMapperTests

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel

	namespace {
		namespace test = catapult::test;

		void InsertRandom(utils::KeySet& keys, size_t count) {
			for (auto i = 0u; i < count; ++i)
				keys.insert(test::GenerateRandomData<Key_Size>());
		}

		state::MultisigEntry CreateMultisigEntry(uint8_t numCosignatories, uint8_t numMultisigAccounts) {
			state::MultisigEntry entry(test::GenerateRandomData<Key_Size>());
			entry.setMinApproval(12);
			entry.setMinRemoval(23);

			InsertRandom(entry.cosignatories(), numCosignatories);
			InsertRandom(entry.multisigAccounts(), numMultisigAccounts);

			return entry;
		}

		void AssertCanMapMultisigEntry(uint8_t numCosignatories, uint8_t numMultisigAccounts) {
			// Arrange:
			auto entry = CreateMultisigEntry(numCosignatories, numMultisigAccounts);

			// Act:
			auto document = ToDbModel(entry);
			auto documentView = document.view();

			// Assert:
			EXPECT_EQ(1u, test::GetFieldCount(documentView));

			auto multisigView = documentView["multisig"].get_document().view();
			EXPECT_EQ(5u, test::GetFieldCount(multisigView));
			mongo::test::AssertEqualMultisigData(entry, multisigView);
		}
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithNeitherCosignatoriesNorMultisigAccounts_ModelToDbModel) {
		// Assert:
		AssertCanMapMultisigEntry(0, 0);
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithCosignatoriesButNoMultisigAccounts_ModelToDbModel) {
		// Assert:
		AssertCanMapMultisigEntry(5, 0);
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithoutCosignatoriesButWithMultisigAccounts_ModelToDbModel) {
		// Assert:
		AssertCanMapMultisigEntry(0, 5);
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithCosignatoriesAndWithMultisigAccounts_ModelToDbModel) {
		// Assert:
		AssertCanMapMultisigEntry(4, 5);
	}

	// endregion

	// region ToMultisigEntry

	namespace {
		bsoncxx::document::value CreateDbMultisigEntry(uint8_t numCosignatories, uint8_t numMultisigAccounts) {
			auto descriptor = CreateMultisigEntry(numCosignatories, numMultisigAccounts);
			return ToDbModel(descriptor);
		}

		void AssertCanMapDbMultisigEntry(uint8_t numCosignatories, uint8_t numMultisigAccounts) {
			// Arrange:
			auto dbMultisigEntry = CreateDbMultisigEntry(numCosignatories, numMultisigAccounts);

			// Act:
			auto entry = ToMultisigEntry(dbMultisigEntry);

			// Assert:
			auto view = dbMultisigEntry.view();
			EXPECT_EQ(1u, test::GetFieldCount(view));

			auto multisigView = view["multisig"].get_document().view();
			EXPECT_EQ(5u, test::GetFieldCount(multisigView));
			mongo::test::AssertEqualMultisigData(entry, multisigView);
		}
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithNeitherCosignatoriesNorMultisigAccounts_DbModelToModel) {
		// Assert:
		AssertCanMapDbMultisigEntry(0, 0);
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithCosignatoriesButNoMultisigAccounts_DbModelToModel) {
		// Assert:
		AssertCanMapDbMultisigEntry(5, 0);
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithoutCosignatoriesButWithMultisigAccounts_DbModelToModel) {
		// Assert:
		AssertCanMapDbMultisigEntry(0, 5);
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithCosignatoriesAndWithMultisigAccounts_DbModelToModel) {
		// Assert:
		AssertCanMapDbMultisigEntry(4, 5);
	}

	// endregion
}}}
