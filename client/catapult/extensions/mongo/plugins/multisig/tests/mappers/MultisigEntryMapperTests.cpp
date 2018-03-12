#include "src/mappers/MultisigEntryMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/MultisigMapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MultisigEntryMapperTests

	// region ToDbModel

	namespace {
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
			auto address = test::GenerateRandomData<Address_Decoded_Size>();

			// Act:
			auto document = ToDbModel(entry, address);
			auto documentView = document.view();

			// Assert:
			EXPECT_EQ(1u, test::GetFieldCount(documentView));

			auto multisigView = documentView["multisig"].get_document().view();
			EXPECT_EQ(6u, test::GetFieldCount(multisigView));
			test::AssertEqualMultisigData(entry, address, multisigView);
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
		bsoncxx::document::value CreateDbMultisigEntry(const Address& address, uint8_t numCosignatories, uint8_t numMultisigAccounts) {
			auto descriptor = CreateMultisigEntry(numCosignatories, numMultisigAccounts);
			return ToDbModel(descriptor, address);
		}

		void AssertCanMapDbMultisigEntry(uint8_t numCosignatories, uint8_t numMultisigAccounts) {
			// Arrange:
			auto address = test::GenerateRandomData<Address_Decoded_Size>();
			auto dbMultisigEntry = CreateDbMultisigEntry(address, numCosignatories, numMultisigAccounts);

			// Act:
			auto entry = ToMultisigEntry(dbMultisigEntry);

			// Assert:
			auto view = dbMultisigEntry.view();
			EXPECT_EQ(1u, test::GetFieldCount(view));

			auto multisigView = view["multisig"].get_document().view();
			EXPECT_EQ(6u, test::GetFieldCount(multisigView));
			test::AssertEqualMultisigData(entry, address, multisigView);
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
