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

	// region add

	namespace {
		constexpr auto Filename = "accounts.dat";

		auto GeneratePrivateKeyBuffers(size_t numPrivateKeys) {
			return test::GenerateRandomDataVector<Key>(numPrivateKeys);
		}

		auto PrepareEntries(const crypto::KeyPair& keyPair, const std::vector<Key>& privateKeyBuffers) {
			test::UnlockedTestEntries entries;
			for (const auto& privateKeyBuffer : privateKeyBuffers) {
				auto entry = test::PrepareUnlockedTestEntry(keyPair, privateKeyBuffer);
				entries.emplace(entry);
			}

			return entries;
		}

		auto PrepareEntries(const crypto::KeyPair& keyPair, size_t numEntries) {
			return PrepareEntries(keyPair, GeneratePrivateKeyBuffers(numEntries));
		}

		auto PrepareEntries(size_t numEntries) {
			auto keyPair = test::GenerateKeyPair();
			return PrepareEntries(keyPair, numEntries);
		}
	}

	TEST(TEST_CLASS, AddThrowsWhenEntryHasInvalidSize) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		Key key;
		auto keyPair = test::GenerateKeyPair();
		auto privateKeyBuffer = test::GenerateRandomByteArray<Key>();
		auto entry = test::ConvertUnlockedTestEntryToBuffer(test::PrepareUnlockedTestEntry(keyPair, privateKeyBuffer));
		entry.resize(entry.size() + 1);

		// Act + Assert:
		EXPECT_THROW(storage.add(key, entry, keyPair.publicKey()), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanAddEntryWithProperSize) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		Key harvesterKey;
		auto entries = PrepareEntries(3);

		// Act:
		for (const auto& entry : entries)
			EXPECT_NO_THROW(storage.add(entry.Key, entry.Payload, harvesterKey));

		// Assert:
		test::AssertUnlockedEntriesFileContents(guard.name(), entries);
	}

	// endregion

	// region save

	namespace {
		void CreateFile(const std::string& filename, size_t size = 0) {
			io::RawFile file(filename, io::OpenMode::Read_Write);
			if (0 == size)
				return;

			auto buffer = test::GenerateRandomVector(size);
			file.write(buffer);
		}

		bool NoFiltering(const Key&) {
			return true;
		}

		bool FullFiltering(const Key&) {
			return false;
		}

		auto AddEntries(UnlockedAccountsStorage& storage, const test::UnlockedTestEntries& entries) {
			std::map<Key, Key> announcerToHarvester;
			Key harvesterKey;
			auto i = 0u;
			for (const auto& entry : entries) {
				harvesterKey[0] = static_cast<uint8_t>(i + 1);
				storage.add(entry.Key, entry.Payload, harvesterKey);
				announcerToHarvester.emplace(entry.Key, harvesterKey);
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
		storage.save([](const auto& harvesterKey) { return harvesterKey[0] & 1; });

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
		test::UnlockedTestEntries expectedEntries;
		std::copy_if(entries.cbegin(), entries.cend(), std::inserter(expectedEntries, expectedEntries.end()), [&announcerToHarvester](
				const auto& entry) {
			const auto& harvesterKey = announcerToHarvester[entry.Key];
			return harvesterKey[0] & 1;
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
		auto entries = PrepareEntries(keyPair, privateKeyBuffers);
		for (const auto& entry : entries)
			AppendUnlockedEntryToFile(guard.name(), entry);

		// Act:
		UnlockedAccountsStorage storage(guard.name());
		std::set<Key> collectedKeys;
		storage.load(keyPair, [&collectedKeys](auto&& harvesterKeyPair) {
			collectedKeys.insert(harvesterKeyPair.publicKey());
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
		auto entry = test::PrepareUnlockedTestEntry(keyPair, randomPrivateBuffer);
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
		auto entry = test::PrepareUnlockedTestEntry(keyPair, invalidPrivateBuffer);
		AppendUnlockedEntryToFile(guard.name(), entry);

		// Act + Assert:
		UnlockedAccountsStorage storage(guard.name());
		EXPECT_THROW(storage.load(keyPair, [](const auto&) {}), catapult_runtime_error);
	}

	// endregion
}}
