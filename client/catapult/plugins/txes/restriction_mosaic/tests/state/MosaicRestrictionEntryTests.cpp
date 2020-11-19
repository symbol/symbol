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

#include "src/state/MosaicRestrictionEntry.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/MosaicRestrictionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicRestrictionEntryTests

	// region test utils

	namespace {
#pragma pack(push, 1)

		struct PackedUniqueKey {
			catapult::MosaicId MosaicId;
			catapult::Address Address;
		};

#pragma pack(pop)

		Hash256 CalculateExpectedUniqueKey(MosaicId mosaicId, const Address& address) {
			auto packedUniqueKey = PackedUniqueKey{ mosaicId, address };

			Hash256 uniqueKey;
			crypto::Sha3_256({ reinterpret_cast<const uint8_t*>(&packedUniqueKey), sizeof(PackedUniqueKey) }, uniqueKey);
			return uniqueKey;
		}
	}

	// endregion

	// region MosaicRestrictionEntry - constructor

	TEST(TEST_CLASS, CanCreateEntryAroundAddressRestriction) {
		// Act:
		MosaicRestrictionEntry entry(MosaicAddressRestriction(MosaicId(123), Address{ { 35 } }));

		// Assert:
		EXPECT_EQ(MosaicRestrictionEntry::EntryType::Address, entry.entryType());

		auto expectedUniqueKey = CalculateExpectedUniqueKey(MosaicId(123), Address{ { 35 } });
		EXPECT_EQ(expectedUniqueKey, entry.uniqueKey());

		EXPECT_EQ(MosaicId(123), entry.asAddressRestriction().mosaicId());
		EXPECT_EQ(MosaicId(123), const_cast<const MosaicRestrictionEntry&>(entry).asAddressRestriction().mosaicId());

		EXPECT_THROW(entry.asGlobalRestriction(), catapult_runtime_error);
		EXPECT_THROW(const_cast<const MosaicRestrictionEntry&>(entry).asGlobalRestriction(), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanCreateEntryAroundGlobalRestriction) {
		// Act:
		MosaicRestrictionEntry entry(MosaicGlobalRestriction(MosaicId(123)));

		// Assert:
		EXPECT_EQ(MosaicRestrictionEntry::EntryType::Global, entry.entryType());

		auto expectedUniqueKey = CalculateExpectedUniqueKey(MosaicId(123), Address());
		EXPECT_EQ(expectedUniqueKey, entry.uniqueKey());

		EXPECT_THROW(entry.asAddressRestriction(), catapult_runtime_error);
		EXPECT_THROW(const_cast<const MosaicRestrictionEntry&>(entry).asAddressRestriction(), catapult_runtime_error);

		EXPECT_EQ(MosaicId(123), entry.asGlobalRestriction().mosaicId());
		EXPECT_EQ(MosaicId(123), const_cast<const MosaicRestrictionEntry&>(entry).asGlobalRestriction().mosaicId());
	}

	// endregion

	// region MosaicRestrictionEntry - copy constructor / assignment operator

	namespace {
		template<typename TCopier>
		void AssertCopyWithAddressRestriction(TCopier copier) {
			// Arrange:
			MosaicRestrictionEntry originalEntry(MosaicAddressRestriction(MosaicId(123), Address{ { 35 } }));
			auto uniqueKey = originalEntry.uniqueKey();

			// Act: make a copy and set a restriction
			auto entry = copier(originalEntry);
			entry.asAddressRestriction().set(1, 1);

			// Assert: properties are correct
			EXPECT_EQ(MosaicRestrictionEntry::EntryType::Address, entry.entryType());
			EXPECT_EQ(uniqueKey, entry.uniqueKey());

			// - restriction is only set in copy
			EXPECT_EQ(0u, originalEntry.asAddressRestriction().size());
			EXPECT_EQ(1u, entry.asAddressRestriction().size());
		}

		template<typename TCopier>
		void AssertCopyWithGlobalRestriction(TCopier copier) {
			// Arrange:
			MosaicRestrictionEntry originalEntry(MosaicGlobalRestriction(MosaicId(123)));
			auto uniqueKey = originalEntry.uniqueKey();

			// Act: make a copy and set a restriction
			auto entry = copier(originalEntry);
			entry.asGlobalRestriction().set(1, { MosaicId(), 1, model::MosaicRestrictionType::EQ });

			// Assert: properties are correct
			EXPECT_EQ(MosaicRestrictionEntry::EntryType::Global, entry.entryType());
			EXPECT_EQ(uniqueKey, entry.uniqueKey());

			// - restriction is only set in copy
			EXPECT_EQ(0u, originalEntry.asGlobalRestriction().size());
			EXPECT_EQ(1u, entry.asGlobalRestriction().size());
		}
	}

	TEST(TEST_CLASS, CanCopyEntryContainingAddressRestriction) {
		AssertCopyWithAddressRestriction([](const auto& originalEntry) {
			// Act:
			return MosaicRestrictionEntry(originalEntry);
		});
	}

	TEST(TEST_CLASS, CanCopyEntryContainingGlobalRestriction) {
		AssertCopyWithGlobalRestriction([](const auto& originalEntry) {
			// Act:
			return MosaicRestrictionEntry(originalEntry);
		});
	}

	TEST(TEST_CLASS, CanAssignEntryContainingAddressRestriction) {
		AssertCopyWithAddressRestriction([](const auto& originalEntry) {
			// Act:
			MosaicRestrictionEntry entry(MosaicGlobalRestriction(MosaicId(0)));
			const auto& result = (entry = originalEntry);

			// Assert:
			EXPECT_EQ(&entry, &result);
			return entry;
		});
	}

	TEST(TEST_CLASS, CanAssignEntryContainingGlobalRestriction) {
		AssertCopyWithGlobalRestriction([](const auto& originalEntry) {
			// Act:
			MosaicRestrictionEntry entry(MosaicAddressRestriction(MosaicId(0), Address()));
			const auto& result = (entry = originalEntry);

			// Assert:
			EXPECT_EQ(&entry, &result);
			return entry;
		});
	}

	// endregion

	// region CreateMosaicRestrictionEntryKey

	TEST(TEST_CLASS, CanCreateMosaicRestrictionEntryKeyFromMosaicId) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();

		// Act:
		auto key = CreateMosaicRestrictionEntryKey(mosaicId);

		// Assert:
		auto expectedUniqueKey = CalculateExpectedUniqueKey(mosaicId, Address());
		EXPECT_EQ(expectedUniqueKey, key);
	}

	TEST(TEST_CLASS, CanCreateMosaicRestrictionEntryKeyFromMosaicIdAndAddress) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		auto address = test::GenerateRandomByteArray<Address>();

		// Act:
		auto key = CreateMosaicRestrictionEntryKey(mosaicId, address);

		// Assert:
		auto expectedUniqueKey = CalculateExpectedUniqueKey(mosaicId, address);
		EXPECT_EQ(expectedUniqueKey, key);
	}

	// endregion
}}
