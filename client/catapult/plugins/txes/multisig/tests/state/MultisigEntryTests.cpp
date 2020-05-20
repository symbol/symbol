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

		void AssertCosignatories(const std::vector<Address>& expectedCosignatories, const MultisigEntry& entry) {
			test::AssertContents(expectedCosignatories, entry.cosignatoryAddresses());
		}

		void AssertMultisigAccounts(const std::vector<Address>& expectedMultisigAccounts, const MultisigEntry& entry) {
			test::AssertContents(expectedMultisigAccounts, entry.multisigAddresses());
		}
	}

	TEST(TEST_CLASS, CanCreateMultisigEntry) {
		// Act:
		auto address = test::GenerateRandomByteArray<Address>();
		auto entry = MultisigEntry(address);

		// Assert:
		EXPECT_EQ(address, entry.address());

		AssertSettings(0, 0, entry);
		AssertCosignatories({}, entry);
		AssertMultisigAccounts({}, entry);
	}

	TEST(TEST_CLASS, CanChangeSettings) {
		// Arrange:
		auto address = test::GenerateRandomByteArray<Address>();
		auto entry = MultisigEntry(address);

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
		auto address = test::GenerateRandomByteArray<Address>();
		auto accountAddresses = test::GenerateRandomDataVector<Address>(10);
		auto entry = MultisigEntry(address);

		// Act:
		entry.setMinApproval(55);
		entry.setMinRemoval(66);

		decltype(accountAddresses) expectedCosignatories;
		for (auto i = 0u; i < 10; ++i)
			entry.cosignatoryAddresses().insert(accountAddresses[i]);

		for (auto i = 0u; i < 10; i += 2)
			entry.cosignatoryAddresses().erase(accountAddresses[i]);

		for (auto i = 1u; i < 10; i += 2)
			expectedCosignatories.push_back(accountAddresses[i]);

		decltype(accountAddresses) expectedMultisigAccounts;
		for (auto i = 0u; i < 10; ++i)
			entry.multisigAddresses().insert(accountAddresses[i]);

		for (auto i = 1u; i < 10; i += 2)
			entry.multisigAddresses().erase(accountAddresses[i]);

		for (auto i = 0u; i < 10; i += 2)
			expectedMultisigAccounts.push_back(accountAddresses[i]);

		// Assert:
		AssertSettings(55, 66, entry);
		AssertCosignatories(expectedCosignatories, entry);
		AssertMultisigAccounts(expectedMultisigAccounts, entry);
	}

	TEST(TEST_CLASS, HasCosignatoryReturnsTrueWhenAddressIsCosignatory) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto entry = MultisigEntry(test::GenerateRandomByteArray<Address>());

		for (auto i = 0u; i < 10; ++i)
			entry.cosignatoryAddresses().insert(addresses[i]);

		// Act + Assert:
		auto i = 0u;
		for (const auto& address : addresses) {
			EXPECT_TRUE(entry.hasCosignatory(address)) << "address " << i;
			++i;
		}
	}

	TEST(TEST_CLASS, HasCosignatoryReturnsFalseWhenAddressIsMultisig) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto entry = MultisigEntry(test::GenerateRandomByteArray<Address>());

		for (auto i = 0u; i < 10; ++i) {
			entry.cosignatoryAddresses().insert(test::GenerateRandomByteArray<Address>());
			entry.multisigAddresses().insert(addresses[i]);
		}

		// Act + Assert:
		auto i = 0u;
		for (const auto& address : addresses) {
			EXPECT_FALSE(entry.hasCosignatory(address)) << "address " << i;
			++i;
		}
	}

	TEST(TEST_CLASS, HasCosignatoryReturnsFalseWhenAddressIsOther) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(10);
		auto entry = MultisigEntry(test::GenerateRandomByteArray<Address>());

		for (auto i = 0u; i < 10; ++i)
			entry.cosignatoryAddresses().insert(test::GenerateRandomByteArray<Address>());

		// Act + Assert:
		auto i = 0u;
		for (const auto& address : addresses) {
			EXPECT_FALSE(entry.hasCosignatory(address)) << "address " << i;
			++i;
		}
	}
}}
