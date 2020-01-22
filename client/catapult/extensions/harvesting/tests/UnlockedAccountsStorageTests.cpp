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

#include "harvesting/src/UnlockedAccountsStorage.h"
#include "harvesting/src/UnlockedFileQueueConsumer.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/io/BufferedFileStream.h"
#include "harvesting/tests/test/UnlockedTestEntry.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace harvesting {

#define TEST_CLASS UnlockedAccountsStorageTests

	namespace {
		constexpr auto Filename = "accounts.dat";

		// region test utils

		auto GeneratePrivateKeyBuffers(size_t numPrivateKeys) {
			return test::GenerateRandomDataVector<Key>(numPrivateKeys);
		}

		auto PrepareEntries(const Key& recipientPublicKey, const std::vector<Key>& privateKeyBuffers) {
			test::UnlockedTestEntries entries;
			for (const auto& privateKeyBuffer : privateKeyBuffers) {
				auto entry = test::PrepareUnlockedTestEntry(recipientPublicKey, privateKeyBuffer);
				entries.emplace(entry);
			}

			return entries;
		}

		auto PrepareEntries(const Key& recipientPublicKey, size_t numEntries) {
			return PrepareEntries(recipientPublicKey, GeneratePrivateKeyBuffers(numEntries));
		}

		auto PrepareEntries(size_t numEntries) {
			auto keyPair = test::GenerateKeyPair();
			return PrepareEntries(keyPair.publicKey(), numEntries);
		}

		void AssertFileSize(const std::string& filename, size_t expectedSize) {
			io::RawFile file(filename, io::OpenMode::Read_Only);
			EXPECT_EQ(expectedSize, file.size());
		}

		auto SeedEntries(
				UnlockedAccountsStorage& storage,
				const std::string& filename,
				size_t numNewEntries,
				size_t numInitialEntries = 0) {
			auto entries = PrepareEntries(numNewEntries);

			for (const auto& entry : entries)
				EXPECT_NO_THROW(storage.add(entry.Key, entry.Payload, test::GenerateRandomByteArray<Key>()));

			// Sanity:
			AssertFileSize(filename, (numNewEntries + numInitialEntries) * sizeof(test::UnlockedTestEntry));
			return entries;
		}

		// endregion
	}

	// region containsAnnouncer

	TEST(TEST_CLASS, ContainsAnnouncerReturnsTrueForKnownAnnouncer) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = SeedEntries(storage, guard.name(), 3);

		// Act + Assert:
		for (const auto& entry : entries)
			EXPECT_TRUE(storage.containsAnnouncer(entry.Key));
	}

	TEST(TEST_CLASS, ContainsAnnouncerReturnsFalseForUnknownAnnouncer) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = SeedEntries(storage, guard.name(), 3);

		// Act + Assert:
		for (auto i = 0u; i < 10u; ++i)
			EXPECT_FALSE(storage.containsAnnouncer(test::GenerateRandomByteArray<Key>()));
	}

	// endregion

	// region add

	TEST(TEST_CLASS, AddThrowsWhenEntryHasInvalidSize) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto keyPair = test::GenerateKeyPair();
		auto privateKeyBuffer = test::GenerateRandomByteArray<Key>();
		auto entry = test::ConvertUnlockedTestEntryToBuffer(test::PrepareUnlockedTestEntry(keyPair.publicKey(), privateKeyBuffer));
		entry.resize(entry.size() + 1);

		// Act + Assert:
		Key entryPublicKey;
		EXPECT_THROW(storage.add(entryPublicKey, entry, keyPair.publicKey()), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanAddEntriesWithProperSize) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());

		// Act:
		auto entries = SeedEntries(storage, guard.name(), 3);

		// Assert:
		test::AssertUnlockedEntriesFileContents(guard.name(), entries);
	}

	TEST(TEST_CLASS, CanAddMultipleEntriesWithSameHarvester) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = PrepareEntries(3);
		auto harvesterPublicKey = test::GenerateRandomByteArray<Key>();

		// Act:
		for (const auto& entry : entries)
			EXPECT_NO_THROW(storage.add(entry.Key, entry.Payload, harvesterPublicKey));

		// Assert:
		test::AssertUnlockedEntriesFileContents(guard.name(), entries);
	}

	TEST(TEST_CLASS, CannotAddMultipleEntriesWithSameAnnouncer) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entry1 = *PrepareEntries(1).cbegin();
		auto entry2 = entry1;

		// Act:
		EXPECT_NO_THROW(storage.add(entry1.Key, entry1.Payload, test::GenerateRandomByteArray<Key>()));
		EXPECT_THROW(storage.add(entry2.Key, entry2.Payload, test::GenerateRandomByteArray<Key>()), catapult_invalid_argument);

		// Assert: the second entry with a duplicate announcer was not added
		test::AssertUnlockedEntriesFileContents(guard.name(), { entry1 });
	}

	// endregion

	// region remove

	namespace {
		void AssertRemoveThrowsWhenInputHasIncompleteEntries(size_t numEntries) {
			// Arrange:
			test::TempFileGuard guard(Filename);
			UnlockedAccountsStorage storage(guard.name());
			auto entries = SeedEntries(storage, guard.name(), numEntries);
			boost::filesystem::resize_file(guard.name(), numEntries * sizeof(test::UnlockedTestEntry) - 1);

			// Sanity:
			EXPECT_TRUE(boost::filesystem::exists(guard.name()));

			// Act + Assert:
			EXPECT_THROW(storage.remove(entries.cbegin()->Key), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, RemoveThrowsWhenInputDoesNotHaveEnoughData) {
		AssertRemoveThrowsWhenInputHasIncompleteEntries(1);
	}

	TEST(TEST_CLASS, RemoveThrowsWhenInputHasIncompleteData) {
		AssertRemoveThrowsWhenInputHasIncompleteEntries(3);
	}

	TEST(TEST_CLASS, RemoveCanRemoveAnnouncerFromMapWhenFileHasBeenDeleted) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = SeedEntries(storage, guard.name(), 1);
		boost::filesystem::remove(guard.name());

		// Sanity:
		const auto& announcerPublicKey = entries.cbegin()->Key;
		EXPECT_TRUE(storage.containsAnnouncer(announcerPublicKey));
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));

		// Act:
		storage.remove(announcerPublicKey);

		// Assert: map but not file was updated
		EXPECT_FALSE(storage.containsAnnouncer(announcerPublicKey));
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));
	}

	TEST(TEST_CLASS, RemoveCanRemoveAnnouncerFromMapWhenAnnouncerHasAlreadyBeenRemovedFromFile) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = SeedEntries(storage, guard.name(), 3);
		boost::filesystem::resize_file(guard.name(), 2 * sizeof(test::UnlockedTestEntry));

		// Sanity:
		const auto& announcerPublicKey = (--entries.cend())->Key;
		EXPECT_TRUE(storage.containsAnnouncer(announcerPublicKey));
		AssertFileSize(guard.name(), 2 * sizeof(test::UnlockedTestEntry));

		// Act:
		storage.remove(announcerPublicKey);

		// Assert: map but not file was updated
		EXPECT_FALSE(storage.containsAnnouncer(announcerPublicKey));
		AssertFileSize(guard.name(), 2 * sizeof(test::UnlockedTestEntry));
	}

	TEST(TEST_CLASS, RemoveCanRemoveAnnouncerFromMapAndThenFromFile) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = SeedEntries(storage, guard.name(), 3);

		// Sanity:
		const auto& announcerPublicKey2 = (++entries.cbegin())->Key;
		const auto& announcerPublicKey3 = (++++entries.cbegin())->Key;
		EXPECT_TRUE(storage.containsAnnouncer(announcerPublicKey2));
		EXPECT_TRUE(storage.containsAnnouncer(announcerPublicKey3));
		AssertFileSize(guard.name(), 3 * sizeof(test::UnlockedTestEntry));

		// Act: remove the second key
		storage.remove(announcerPublicKey2);

		// Assert: map but not file was updated (last key in file is announcerPublicKey3)
		EXPECT_FALSE(storage.containsAnnouncer(announcerPublicKey2));
		EXPECT_TRUE(storage.containsAnnouncer(announcerPublicKey3));
		AssertFileSize(guard.name(), 3 * sizeof(test::UnlockedTestEntry));

		// Act: remove the third key
		storage.remove(announcerPublicKey3);

		// Assert: map and file were updated
		EXPECT_FALSE(storage.containsAnnouncer(announcerPublicKey2));
		EXPECT_FALSE(storage.containsAnnouncer(announcerPublicKey3));
		AssertFileSize(guard.name(), 2 * sizeof(test::UnlockedTestEntry));

		// Act: remove the second key (again)
		storage.remove(announcerPublicKey2);

		// Assert: file but not map was updated
		EXPECT_FALSE(storage.containsAnnouncer(announcerPublicKey2));
		EXPECT_FALSE(storage.containsAnnouncer(announcerPublicKey3));
		AssertFileSize(guard.name(), 1 * sizeof(test::UnlockedTestEntry));
	}

	TEST(TEST_CLASS, CanRemoveAllEntriesInReverseOrder) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = SeedEntries(storage, guard.name(), 5);

		// Act:
		for (auto iter = entries.crbegin(); entries.crend() != iter; ++iter)
			storage.remove(iter->Key);

		// Assert:
		test::AssertUnlockedEntriesFileContents(guard.name(), {});
	}

	TEST(TEST_CLASS, CanRemoveSomeEntriesInReverseOrder) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = SeedEntries(storage, guard.name(), 3);
		auto entriesNew = SeedEntries(storage, guard.name(), 2, 3);

		// Act:
		for (auto iter = entriesNew.crbegin(); entriesNew.crend() != iter; ++iter)
			storage.remove(iter->Key);

		// Assert:
		test::AssertUnlockedEntriesFileContents(guard.name(), entries);
	}

	TEST(TEST_CLASS, RemoveIgnoresUnknownAnnouncer) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = SeedEntries(storage, guard.name(), 3);

		// Act:
		for (auto i = 0u; i < 10u; ++i)
			EXPECT_NO_THROW(storage.remove(test::GenerateRandomByteArray<Key>()));

		// Assert:
		test::AssertUnlockedEntriesFileContents(guard.name(), entries);
	}

	// endregion

	// region save

	namespace {
		bool NoFiltering(const Key&) {
			return true;
		}

		bool FullFiltering(const Key&) {
			return false;
		}

		void CreateFile(const std::string& filename, size_t size = 0) {
			io::RawFile file(filename, io::OpenMode::Read_Write);
			if (0 == size)
				return;

			auto buffer = test::GenerateRandomVector(size);
			file.write(buffer);
		}

		auto AddEntries(UnlockedAccountsStorage& storage, const test::UnlockedTestEntries& entries) {
			std::map<Key, Key> announcerToHarvester;
			Key harvesterPublicKey;
			auto i = 0u;
			for (const auto& entry : entries) {
				harvesterPublicKey[0] = static_cast<uint8_t>(i + 1);
				storage.add(entry.Key, entry.Payload, harvesterPublicKey);
				announcerToHarvester.emplace(entry.Key, harvesterPublicKey);
				++i;
			}

			return announcerToHarvester;
		}
	}

	TEST(TEST_CLASS, SaveStoresAddedUnlockedEntriesWhenNoFilteringIsApplied) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = PrepareEntries(3);
		AddEntries(storage, entries);

		// Act:
		storage.save(NoFiltering);

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
		test::AssertUnlockedEntriesFileContents(guard.name(), entries);
	}

	TEST(TEST_CLASS, SaveRemovesExistingFileWhenUnlockedEntriesAreEmpty) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		CreateFile(guard.name());

		UnlockedAccountsStorage storage(guard.name());

		// Sanity:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));

		// Act:
		storage.save(NoFiltering);

		// Assert:
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));
	}

	TEST(TEST_CLASS, SaveRemovesExistingFileWhenAllUnlockedEntriesAreFiltered) {
		// Arrange: create file
		test::TempFileGuard guard(Filename);
		CreateFile(guard.name());

		UnlockedAccountsStorage storage(guard.name());
		auto entries = PrepareEntries(3);
		AddEntries(storage, entries);

		// Sanity:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));

		// Act:
		storage.save(FullFiltering);

		// Assert:
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));
	}

	TEST(TEST_CLASS, SaveFiltersUnlockedEntries) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto entries = PrepareEntries(5);
		auto announcerToHarvester = AddEntries(storage, entries);

		// Act:
		storage.save([](const auto& harvesterPublicKey) { return harvesterPublicKey[0] & 1; });

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
		test::UnlockedTestEntries expectedEntries;
		std::copy_if(entries.cbegin(), entries.cend(), std::inserter(expectedEntries, expectedEntries.end()), [&announcerToHarvester](
				const auto& entry) {
			const auto& harvesterPublicKey = announcerToHarvester[entry.Key];
			return harvesterPublicKey[0] & 1;
		});

		// - every odd entry should be in expected set
		EXPECT_EQ(3u, expectedEntries.size());
		test::AssertUnlockedEntriesFileContents(guard.name(), expectedEntries);
	}

	namespace {
		auto CreateFileWithEntries(const std::string& filename, size_t numEntries) {
			UnlockedAccountsStorage storage(filename);
			auto entries = PrepareEntries(numEntries);
			AddEntries(storage, entries);

			storage.save(NoFiltering);
			return entries;
		}
	}

	TEST(TEST_CLASS, SaveOverwritesExistingFile) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		auto originalEntries = CreateFileWithEntries(guard.name(), 3);

		// Sanity:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
		test::AssertUnlockedEntriesFileContents(guard.name(), originalEntries);

		// - add new entries
		UnlockedAccountsStorage storage(guard.name());
		auto entries = PrepareEntries(5);
		AddEntries(storage, entries);

		// Act:
		storage.save(NoFiltering);

		// Assert: there was no load(), so originalEntries should be overwritten with entries
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
		test::AssertUnlockedEntriesFileContents(guard.name(), entries);
	}

	TEST(TEST_CLASS, SaveDoesNotSaveRemovedEntries) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto originalEntries = PrepareEntries(5);
		AddEntries(storage, originalEntries);

		storage.save(NoFiltering);

		// - remove two most recent entries
		auto reverseIter = originalEntries.crbegin();
		for (int i = 0; i < 2; ++i) {
			storage.remove(reverseIter->Key);
			++reverseIter;
		}

		// Act:
		storage.save(NoFiltering);

		// Assert: only first three entries should be present
		auto iter = originalEntries.cbegin();
		std::advance(iter, 3);
		originalEntries.erase(iter, originalEntries.cend());

		test::AssertUnlockedEntriesFileContents(guard.name(), originalEntries);
	}

	// endregion

	// region load

	TEST(TEST_CLASS, CanLoadWhenInputIsNotPresent) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		auto keyPair = test::GenerateKeyPair();

		// Sanity:
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));

		// Act + Assert:
		UnlockedAccountsStorage storage(guard.name());
		EXPECT_NO_THROW(storage.load(keyPair, [](const auto&) {}));
	}

	TEST(TEST_CLASS, CanLoadWhenInputIsEmpty) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		CreateFile(guard.name());
		auto keyPair = test::GenerateKeyPair();

		// Act + Assert:
		UnlockedAccountsStorage storage(guard.name());
		EXPECT_NO_THROW(storage.load(keyPair, [](const auto&) {}));
	}

	TEST(TEST_CLASS, CannotLoadWhenInputDoesNotHaveEnoughData) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		CreateFile(guard.name(), sizeof(test::UnlockedTestEntry) - 1);
		auto keyPair = test::GenerateKeyPair();

		// Act + Assert:
		UnlockedAccountsStorage storage(guard.name());
		EXPECT_THROW(storage.load(keyPair, [](const auto&) {}), catapult_file_io_error);
	}

	namespace {
		auto AppendUnlockedEntryToFile(const std::string& filename, const test::UnlockedTestEntry& entry) {
			io::RawFile output(filename, io::OpenMode::Read_Append);
			output.seek(output.size());
			output.write({ reinterpret_cast<const uint8_t*>(&entry), sizeof(test::UnlockedTestEntry) });
			return entry;
		}

		void AppendToFile(const std::string& filename, size_t size) {
			io::RawFile file(filename, io::OpenMode::Read_Append);
			file.seek(file.size());
			auto buffer = test::GenerateRandomVector(size);
			file.write(buffer);
		}
	}

	TEST(TEST_CLASS, CanLoadUnlockedEntries) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		auto keyPair = test::GenerateKeyPair();
		auto privateKeyBuffers = GeneratePrivateKeyBuffers(3);
		auto entries = PrepareEntries(keyPair.publicKey(), privateKeyBuffers);
		for (const auto& entry : entries)
			AppendUnlockedEntryToFile(guard.name(), entry);

		// Act:
		UnlockedAccountsStorage storage(guard.name());
		std::set<Key> collectedKeys;
		storage.load(keyPair, [&collectedKeys](auto&& harvesterPublicKeyPair) {
			collectedKeys.insert(harvesterPublicKeyPair.publicKey());
		});

		// Assert: remove the file, save and check if it matches initial data
		boost::filesystem::remove(guard.name());
		storage.save(NoFiltering);
		test::AssertUnlockedEntriesFileContents(guard.name(), entries);

		// - check all loaded keys were passed to process function
		std::set<Key> expectedKeys;
		for (const auto& privateKeyBuffer : privateKeyBuffers)
			expectedKeys.insert(test::KeyPairFromPrivateKeyBuffer(privateKeyBuffer).publicKey());

		EXPECT_EQ(expectedKeys, collectedKeys);
	}

	TEST(TEST_CLASS, LoadThrowsWhenInputDoesNotHaveEnoughData) {
		// Arrange: create file with one proper entry and one "short"
		test::TempFileGuard guard(Filename);
		auto keyPair = test::GenerateKeyPair();
		auto randomPrivateBuffer = test::GenerateRandomByteArray<Key>();
		auto entry = test::PrepareUnlockedTestEntry(keyPair.publicKey(), randomPrivateBuffer);
		AppendUnlockedEntryToFile(guard.name(), entry);
		AppendToFile(guard.name(), sizeof(test::UnlockedTestEntry) - 1);

		// Act + Assert:
		UnlockedAccountsStorage storage(guard.name());
		EXPECT_THROW(storage.load(keyPair, [](const auto&) {}), catapult_file_io_error);
	}

	TEST(TEST_CLASS, LoadThrowsWhenDataIsInvalid) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		auto keyPair = test::GenerateKeyPair();
		auto invalidPrivateBuffer = test::GenerateRandomArray<Key::Size + 1>();
		auto entry = test::PrepareUnlockedTestEntry(keyPair.publicKey(), invalidPrivateBuffer);
		AppendUnlockedEntryToFile(guard.name(), entry);

		// Act + Assert:
		UnlockedAccountsStorage storage(guard.name());
		EXPECT_THROW(storage.load(keyPair, [](const auto&) {}), catapult_runtime_error);
	}

	// endregion
}}
