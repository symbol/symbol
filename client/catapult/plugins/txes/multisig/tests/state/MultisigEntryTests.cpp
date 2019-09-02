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

#include "src/state/MultisigEntry.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MultisigEntryTests

	namespace {
		void AssertSettings(uint8_t expectedMinApproval, uint8_t expectedMinRemoval, const MultisigEntry& entry) {
			EXPECT_EQ(expectedMinApproval, entry.minApproval());
			EXPECT_EQ(expectedMinRemoval, entry.minRemoval());
		}

		void AssertCosignatories(const std::vector<Key>& expectedCosignatories, const MultisigEntry& entry) {
			test::AssertContents(expectedCosignatories, entry.cosignatoryPublicKeys());
		}

		void AssertMultisigAccounts(const std::vector<Key>& expectedMultisigAccounts, const MultisigEntry& entry) {
			test::AssertContents(expectedMultisigAccounts, entry.multisigPublicKeys());
		}
	}

	TEST(TEST_CLASS, CanCreateMultisigEntry) {
		// Act:
		auto key = test::GenerateRandomByteArray<Key>();
		auto entry = MultisigEntry(key);

		// Assert:
		EXPECT_EQ(key, entry.key());

		AssertSettings(0, 0, entry);
		AssertCosignatories({}, entry);
		AssertMultisigAccounts({}, entry);
	}

	TEST(TEST_CLASS, CanChangeSettings) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto entry = MultisigEntry(key);

		// Act:
		entry.setMinApproval(12);
		entry.setMinRemoval(34);

		// Assert:
		AssertSettings(12, 34, entry);
		AssertCosignatories({}, entry);
		AssertMultisigAccounts({}, entry);
	}

	TEST(TEST_CLASS, CanAddAndRemoveBothCosignatoriesAndAccounts) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto accountKeys = test::GenerateKeys(10);
		auto entry = MultisigEntry(key);

		// Act:
		entry.setMinApproval(55);
		entry.setMinRemoval(66);

		decltype(accountKeys) expectedCosignatories;
		for (auto i = 0u; i < 10; ++i)
			entry.cosignatoryPublicKeys().insert(accountKeys[i]);

		for (auto i = 0u; i < 10; i += 2)
			entry.cosignatoryPublicKeys().erase(accountKeys[i]);

		for (auto i = 1u; i < 10; i += 2)
			expectedCosignatories.push_back(accountKeys[i]);

		decltype(accountKeys) expectedMultisigAccounts;
		for (auto i = 0u; i < 10; ++i)
			entry.multisigPublicKeys().insert(accountKeys[i]);

		for (auto i = 1u; i < 10; i += 2)
			entry.multisigPublicKeys().erase(accountKeys[i]);

		for (auto i = 0u; i < 10; i += 2)
			expectedMultisigAccounts.push_back(accountKeys[i]);

		// Assert:
		AssertSettings(55, 66, entry);
		AssertCosignatories(expectedCosignatories, entry);
		AssertMultisigAccounts(expectedMultisigAccounts, entry);
	}

	TEST(TEST_CLASS, HasCosignatoryReturnsTrueWhenKeyIsCosignatory) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto accountKeys = test::GenerateKeys(10);
		auto entry = MultisigEntry(key);

		for (auto i = 0u; i < 10; ++i)
			entry.cosignatoryPublicKeys().insert(accountKeys[i]);

		// Act + Assert:
		auto i = 0u;
		for (const auto& accountKey : accountKeys) {
			EXPECT_TRUE(entry.hasCosignatory(accountKey)) << "key " << i;
			++i;
		}
	}

	TEST(TEST_CLASS, HasCosignatoryReturnsFalseWhenKeyIsNotCosignatory) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		auto accountKeys = test::GenerateKeys(10);
		auto entry = MultisigEntry(key);

		for (auto i = 0u; i < 10; ++i)
			entry.multisigPublicKeys().insert(accountKeys[i]);

		// Act + Assert:
		auto i = 0u;
		for (const auto& accountKey : accountKeys) {
			EXPECT_FALSE(entry.hasCosignatory(accountKey)) << "key " << i;
			++i;
		}
	}
}}
