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

#include "src/mappers/MultisigEntryMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/MultisigMapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MultisigEntryMapperTests

	namespace {
		void InsertRandom(state::SortedAddressSet& addresses, size_t count) {
			for (auto i = 0u; i < count; ++i)
				addresses.insert(test::GenerateRandomByteArray<Address>());
		}

		state::MultisigEntry CreateMultisigEntry(uint8_t numCosignatories, uint8_t numMultisigAccounts) {
			state::MultisigEntry entry(test::GenerateRandomByteArray<Address>());
			entry.setMinApproval(12);
			entry.setMinRemoval(23);

			InsertRandom(entry.cosignatoryAddresses(), numCosignatories);
			InsertRandom(entry.multisigAddresses(), numMultisigAccounts);

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
			test::AssertEqualMultisigData(entry, multisigView);
		}
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithNeitherCosignatoriesNorMultisigAccounts_ModelToDbModel) {
		AssertCanMapMultisigEntry(0, 0);
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithCosignatoriesButNoMultisigAccounts_ModelToDbModel) {
		AssertCanMapMultisigEntry(5, 0);
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithoutCosignatoriesButWithMultisigAccounts_ModelToDbModel) {
		AssertCanMapMultisigEntry(0, 5);
	}

	TEST(TEST_CLASS, CanMapMultisigEntryWithCosignatoriesAndWithMultisigAccounts_ModelToDbModel) {
		AssertCanMapMultisigEntry(4, 5);
	}
}}}
