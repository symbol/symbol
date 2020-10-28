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
#include "catapult/io/BufferedFileStream.h"
#include "harvesting/tests/test/HarvestRequestEncryptedPayload.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace harvesting {

#define TEST_CLASS UnlockedAccountsStorageTests

	namespace {
		constexpr auto Filename = "accounts.dat";

		// region test utils

		auto PrepareEncryptedPayloads(const Key& recipientPublicKey, const std::vector<BlockGeneratorAccountDescriptor>& descriptors) {
			test::HarvestRequestEncryptedPayloads encryptedPayloads;
			for (const auto& descriptor : descriptors) {
				auto encryptedPayload = test::PrepareHarvestRequestEncryptedPayload(
						recipientPublicKey,
						test::ToClearTextBuffer(descriptor));
				encryptedPayloads.emplace(encryptedPayload);
			}

			return encryptedPayloads;
		}

		auto PrepareEncryptedPayloads(const Key& recipientPublicKey, size_t numEncryptedPayloads) {
			return PrepareEncryptedPayloads(recipientPublicKey, test::GenerateRandomAccountDescriptors(numEncryptedPayloads));
		}

		auto PrepareEncryptedPayloads(size_t numEncryptedPayloads) {
			auto keyPair = test::GenerateKeyPair();
			return PrepareEncryptedPayloads(keyPair.publicKey(), numEncryptedPayloads);
		}

		void AssertFileSize(const std::string& filename, size_t expectedSize) {
			io::RawFile file(filename, io::OpenMode::Read_Only);
			EXPECT_EQ(expectedSize, file.size());
		}

		auto SeedEncryptedPayloads(
				UnlockedAccountsStorage& storage,
				const std::string& filename,
				size_t numNewEncryptedPayloads,
				size_t numInitialEncryptedPayloads = 0) {
			auto encryptedPayloads = PrepareEncryptedPayloads(numNewEncryptedPayloads);

			for (const auto& encryptedPayload : encryptedPayloads) {
				auto requestIdentifier = test::GetRequestIdentifier(encryptedPayload);
				EXPECT_NO_THROW(storage.add(requestIdentifier, encryptedPayload.Data, test::GenerateRandomByteArray<Key>()));
			}

			// Sanity:
			auto expectedFileSize = (numNewEncryptedPayloads + numInitialEncryptedPayloads) * sizeof(test::HarvestRequestEncryptedPayload);
			AssertFileSize(filename, expectedFileSize);
			return encryptedPayloads;
		}

		// endregion
	}

	// region contains

	TEST(TEST_CLASS, ContainsReturnsTrueForKnownIdentifier) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 3);

		// Act + Assert:
		for (const auto& encryptedPayload : encryptedPayloads)
			EXPECT_TRUE(storage.contains(test::GetRequestIdentifier(encryptedPayload)));
	}

	TEST(TEST_CLASS, ContainsReturnsFalseForUnknownIdentifier) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 3);

		// Act + Assert:
		for (auto i = 0u; i < 10u; ++i)
			EXPECT_FALSE(storage.contains(test::GenerateRandomByteArray<HarvestRequestIdentifier>()));
	}

	// endregion

	// region add

	TEST(TEST_CLASS, AddThrowsWhenRequestHasInvalidSize) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto keyPair = test::GenerateKeyPair();
		auto decryptedPayload = test::ToClearTextBuffer(test::GenerateRandomAccountDescriptors(1)[0]);
		auto encryptedPayload = test::PrepareHarvestRequestEncryptedPayload(keyPair.publicKey(), decryptedPayload);
		auto encryptedBuffer = test::CopyHarvestRequestEncryptedPayloadToBuffer(encryptedPayload);
		encryptedBuffer.resize(encryptedBuffer.size() + 1);

		// Act + Assert:
		HarvestRequestIdentifier requestIdentifier;
		EXPECT_THROW(storage.add(requestIdentifier, encryptedBuffer, keyPair.publicKey()), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanAddRequestsWithProperSize) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());

		// Act:
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 3);

		// Assert:
		test::AssertHarvesterFileContents(guard.name(), encryptedPayloads);
	}

	TEST(TEST_CLASS, CanAddMultipleRequestsWithSameHarvester) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = PrepareEncryptedPayloads(3);
		auto harvesterPublicKey = test::GenerateRandomByteArray<Key>();

		// Act:
		for (const auto& encryptedPayload : encryptedPayloads)
			EXPECT_NO_THROW(storage.add(test::GetRequestIdentifier(encryptedPayload), encryptedPayload.Data, harvesterPublicKey));

		// Assert:
		test::AssertHarvesterFileContents(guard.name(), encryptedPayloads);
	}

	TEST(TEST_CLASS, CannotAddMultipleRequestsWithSameAnnouncer) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayload1 = *PrepareEncryptedPayloads(1).cbegin();
		auto encryptedPayload2 = encryptedPayload1;

		// Act:
		auto requestIdentifier = test::GetRequestIdentifier(encryptedPayload1);
		EXPECT_NO_THROW(storage.add(requestIdentifier, encryptedPayload1.Data, test::GenerateRandomByteArray<Key>()));

		auto requestIdentifier2 = test::GetRequestIdentifier(encryptedPayload2);
		EXPECT_THROW(
				storage.add(requestIdentifier2, encryptedPayload2.Data, test::GenerateRandomByteArray<Key>()),
				catapult_invalid_argument);

		// Assert: encryptedPayload2 with a duplicate announcer was not added
		test::AssertHarvesterFileContents(guard.name(), { encryptedPayload1 });
	}

	// endregion

	// region remove

	namespace {
		void AssertRemoveThrowsWhenInputHasIncompleteEncryptedPayloads(size_t numEncryptedPayloads) {
			// Arrange:
			test::TempFileGuard guard(Filename);
			UnlockedAccountsStorage storage(guard.name());
			auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), numEncryptedPayloads);
			boost::filesystem::resize_file(guard.name(), numEncryptedPayloads * sizeof(test::HarvestRequestEncryptedPayload) - 1);

			// Sanity:
			EXPECT_TRUE(boost::filesystem::exists(guard.name()));

			// Act + Assert:
			EXPECT_THROW(storage.remove(test::GetRequestIdentifier(*encryptedPayloads.cbegin())), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, RemoveThrowsWhenInputDoesNotHaveEnoughData) {
		AssertRemoveThrowsWhenInputHasIncompleteEncryptedPayloads(1);
	}

	TEST(TEST_CLASS, RemoveThrowsWhenInputHasIncompleteData) {
		AssertRemoveThrowsWhenInputHasIncompleteEncryptedPayloads(3);
	}

	TEST(TEST_CLASS, RemoveCanRemoveAnnouncerFromMapWhenFileHasBeenDeleted) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 1);
		boost::filesystem::remove(guard.name());

		// Sanity:
		auto requestIdentifier = test::GetRequestIdentifier(*encryptedPayloads.cbegin());
		EXPECT_TRUE(storage.contains(requestIdentifier));
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));

		// Act:
		storage.remove(requestIdentifier);

		// Assert: map but not file was updated
		EXPECT_FALSE(storage.contains(requestIdentifier));
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));
	}

	TEST(TEST_CLASS, RemoveCanRemoveAnnouncerFromMapWhenAnnouncerHasAlreadyBeenRemovedFromFile) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 3);
		boost::filesystem::resize_file(guard.name(), 2 * sizeof(test::HarvestRequestEncryptedPayload));

		// Sanity:
		auto requestIdentifier = test::GetRequestIdentifier(*--encryptedPayloads.cend());
		EXPECT_TRUE(storage.contains(requestIdentifier));
		AssertFileSize(guard.name(), 2 * sizeof(test::HarvestRequestEncryptedPayload));

		// Act:
		storage.remove(requestIdentifier);

		// Assert: map but not file was updated
		EXPECT_FALSE(storage.contains(requestIdentifier));
		AssertFileSize(guard.name(), 2 * sizeof(test::HarvestRequestEncryptedPayload));
	}

	TEST(TEST_CLASS, RemoveCanRemoveAnnouncerFromMapAndThenFromFile) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 3);

		// Sanity:
		auto requestIdentifier2 = test::GetRequestIdentifier(*++encryptedPayloads.cbegin());
		auto requestIdentifier3 = test::GetRequestIdentifier(*++++encryptedPayloads.cbegin());
		EXPECT_TRUE(storage.contains(requestIdentifier2));
		EXPECT_TRUE(storage.contains(requestIdentifier3));
		AssertFileSize(guard.name(), 3 * sizeof(test::HarvestRequestEncryptedPayload));

		// Act: remove the second key
		storage.remove(requestIdentifier2);

		// Assert: map but not file was updated (last key in file is requestIdentifier3)
		EXPECT_FALSE(storage.contains(requestIdentifier2));
		EXPECT_TRUE(storage.contains(requestIdentifier3));
		AssertFileSize(guard.name(), 3 * sizeof(test::HarvestRequestEncryptedPayload));

		// Act: remove the third key
		storage.remove(requestIdentifier3);

		// Assert: map and file were updated
		EXPECT_FALSE(storage.contains(requestIdentifier2));
		EXPECT_FALSE(storage.contains(requestIdentifier3));
		AssertFileSize(guard.name(), 2 * sizeof(test::HarvestRequestEncryptedPayload));

		// Act: remove the second key (again)
		storage.remove(requestIdentifier2);

		// Assert: file but not map was updated
		EXPECT_FALSE(storage.contains(requestIdentifier2));
		EXPECT_FALSE(storage.contains(requestIdentifier3));
		AssertFileSize(guard.name(), 1 * sizeof(test::HarvestRequestEncryptedPayload));
	}

	TEST(TEST_CLASS, CanRemoveAllRequestsInReverseOrder) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 5);

		// Act:
		for (auto iter = encryptedPayloads.crbegin(); encryptedPayloads.crend() != iter; ++iter)
			storage.remove(test::GetRequestIdentifier(*iter));

		// Assert:
		test::AssertHarvesterFileContents(guard.name(), {});
	}

	TEST(TEST_CLASS, CanRemoveSomeRequestsInReverseOrder) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 3);
		auto encryptedPayloadsNew = SeedEncryptedPayloads(storage, guard.name(), 2, 3);

		// Act:
		for (auto iter = encryptedPayloadsNew.crbegin(); encryptedPayloadsNew.crend() != iter; ++iter)
			storage.remove(test::GetRequestIdentifier(*iter));

		// Assert:
		test::AssertHarvesterFileContents(guard.name(), encryptedPayloads);
	}

	TEST(TEST_CLASS, RemoveIgnoresUnknownAnnouncer) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 3);

		// Act:
		for (auto i = 0u; i < 10u; ++i)
			EXPECT_NO_THROW(storage.remove(test::GenerateRandomByteArray<HarvestRequestIdentifier>()));

		// Assert:
		test::AssertHarvesterFileContents(guard.name(), encryptedPayloads);
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

		auto AddEncryptedPayloads(UnlockedAccountsStorage& storage, const test::HarvestRequestEncryptedPayloads& encryptedPayloads) {
			std::map<HarvestRequestIdentifier, Key> identifierToHarvester;
			Key harvesterPublicKey;
			auto i = 0u;
			for (const auto& encryptedPayload : encryptedPayloads) {
				harvesterPublicKey[0] = static_cast<uint8_t>(i + 1);
				storage.add(test::GetRequestIdentifier(encryptedPayload), encryptedPayload.Data, harvesterPublicKey);
				identifierToHarvester.emplace(test::GetRequestIdentifier(encryptedPayload), harvesterPublicKey);
				++i;
			}

			return identifierToHarvester;
		}
	}

	TEST(TEST_CLASS, SaveStoresAddedRequestsWhenNoFilteringIsApplied) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = PrepareEncryptedPayloads(3);
		AddEncryptedPayloads(storage, encryptedPayloads);

		// Act:
		storage.save(NoFiltering);

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
		test::AssertHarvesterFileContents(guard.name(), encryptedPayloads);
	}

	TEST(TEST_CLASS, SaveRemovesExistingFileWhenRequestsAreEmpty) {
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

	TEST(TEST_CLASS, SaveRemovesExistingFileWhenAllRequestsAreFiltered) {
		// Arrange: create file
		test::TempFileGuard guard(Filename);
		CreateFile(guard.name());

		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = PrepareEncryptedPayloads(3);
		AddEncryptedPayloads(storage, encryptedPayloads);

		// Sanity:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));

		// Act:
		storage.save(FullFiltering);

		// Assert:
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));
	}

	TEST(TEST_CLASS, SaveFiltersRequests) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = PrepareEncryptedPayloads(5);
		auto identifierToHarvester = AddEncryptedPayloads(storage, encryptedPayloads);

		// Act:
		storage.save([](const auto& harvesterPublicKey) { return harvesterPublicKey[0] & 1; });

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
		test::HarvestRequestEncryptedPayloads expectedEncryptedPayloads;
		auto inserter = std::inserter(expectedEncryptedPayloads, expectedEncryptedPayloads.end());
		std::copy_if(encryptedPayloads.cbegin(), encryptedPayloads.cend(), inserter, [&identifierToHarvester](
				const auto& encryptedPayload) {
			const auto& harvesterPublicKey = identifierToHarvester[test::GetRequestIdentifier(encryptedPayload)];
			return harvesterPublicKey[0] & 1;
		});

		// - every odd encryptedPayload should be in expected set
		EXPECT_EQ(3u, expectedEncryptedPayloads.size());
		test::AssertHarvesterFileContents(guard.name(), expectedEncryptedPayloads);
	}

	namespace {
		auto CreateFileWithEncryptedPayloads(const std::string& filename, size_t numEncryptedPayloads) {
			UnlockedAccountsStorage storage(filename);
			auto encryptedPayloads = PrepareEncryptedPayloads(numEncryptedPayloads);
			AddEncryptedPayloads(storage, encryptedPayloads);

			storage.save(NoFiltering);
			return encryptedPayloads;
		}
	}

	TEST(TEST_CLASS, SaveOverwritesExistingFile) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		auto originalEncryptedPayloads = CreateFileWithEncryptedPayloads(guard.name(), 3);

		// Sanity:
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
		test::AssertHarvesterFileContents(guard.name(), originalEncryptedPayloads);

		// - add new encryptedPayloads
		UnlockedAccountsStorage storage(guard.name());
		auto encryptedPayloads = PrepareEncryptedPayloads(5);
		AddEncryptedPayloads(storage, encryptedPayloads);

		// Act:
		storage.save(NoFiltering);

		// Assert: there was no load(), so originalEncryptedPayloads should be overwritten with encryptedPayloads
		EXPECT_TRUE(boost::filesystem::exists(guard.name()));
		test::AssertHarvesterFileContents(guard.name(), encryptedPayloads);
	}

	TEST(TEST_CLASS, SaveDoesNotSaveRemovedRequests) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());
		auto originalEncryptedPayloads = PrepareEncryptedPayloads(5);
		AddEncryptedPayloads(storage, originalEncryptedPayloads);

		storage.save(NoFiltering);

		// - remove two most recent encrypted payloads
		auto reverseIter = originalEncryptedPayloads.crbegin();
		for (int i = 0; i < 2; ++i) {
			storage.remove(test::GetRequestIdentifier(*reverseIter));
			++reverseIter;
		}

		// Act:
		storage.save(NoFiltering);

		// Assert: only first three encrypted payloads should be present
		auto iter = originalEncryptedPayloads.cbegin();
		std::advance(iter, 3);
		originalEncryptedPayloads.erase(iter, originalEncryptedPayloads.cend());

		test::AssertHarvesterFileContents(guard.name(), originalEncryptedPayloads);
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
		CreateFile(guard.name(), sizeof(test::HarvestRequestEncryptedPayload) - 1);
		auto keyPair = test::GenerateKeyPair();

		// Act + Assert:
		UnlockedAccountsStorage storage(guard.name());
		EXPECT_THROW(storage.load(keyPair, [](const auto&) {}), catapult_file_io_error);
	}

	namespace {
		struct BlockGeneratorAccountDescriptorHasher {
			size_t operator()(const BlockGeneratorAccountDescriptor& descriptor) const {
				return utils::ArrayHasher<Key>()(descriptor.signingKeyPair().publicKey());
			}
		};

		using BlockGeneratorAccountDescriptorUnorderedSet =
			std::unordered_set<BlockGeneratorAccountDescriptor, BlockGeneratorAccountDescriptorHasher>;

		auto AppendEncryptedPayloadToFile(const std::string& filename, const test::HarvestRequestEncryptedPayload& encryptedPayload) {
			io::RawFile output(filename, io::OpenMode::Read_Append);
			output.seek(output.size());
			output.write({ reinterpret_cast<const uint8_t*>(&encryptedPayload), sizeof(test::HarvestRequestEncryptedPayload) });
			return encryptedPayload;
		}

		void AppendToFile(const std::string& filename, size_t size) {
			io::RawFile file(filename, io::OpenMode::Read_Append);
			file.seek(file.size());
			auto buffer = test::GenerateRandomVector(size);
			file.write(buffer);
		}
	}

	TEST(TEST_CLASS, CanLoadRequests) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		auto keyPair = test::GenerateKeyPair();
		auto descriptors = test::GenerateRandomAccountDescriptors(3);
		auto encryptedPayloads = PrepareEncryptedPayloads(keyPair.publicKey(), descriptors);
		for (const auto& encryptedPayload : encryptedPayloads)
			AppendEncryptedPayloadToFile(guard.name(), encryptedPayload);

		// Act:
		UnlockedAccountsStorage storage(guard.name());
		BlockGeneratorAccountDescriptorUnorderedSet collectedDescriptors;
		storage.load(keyPair, [&collectedDescriptors](auto&& descriptor) {
			collectedDescriptors.insert(std::move(descriptor));
		});

		// Assert: remove the file, save and check if it matches initial data
		boost::filesystem::remove(guard.name());
		storage.save(NoFiltering);
		test::AssertHarvesterFileContents(guard.name(), encryptedPayloads);

		// - check all loaded keys were passed to process function (order doesn't matter)
		BlockGeneratorAccountDescriptorUnorderedSet expectedDescriptors;
		for (auto& descriptor : descriptors)
			expectedDescriptors.emplace(std::move(descriptor));

		EXPECT_EQ(expectedDescriptors, collectedDescriptors);
	}

	TEST(TEST_CLASS, LoadThrowsWhenInputDoesNotHaveEnoughData) {
		// Arrange: create file with one proper encryptedPayload and one "short"
		test::TempFileGuard guard(Filename);
		auto keyPair = test::GenerateKeyPair();
		auto decryptedPayload = test::ToClearTextBuffer(test::GenerateRandomAccountDescriptors(1)[0]);
		auto encryptedPayload = test::PrepareHarvestRequestEncryptedPayload(keyPair.publicKey(), decryptedPayload);
		AppendEncryptedPayloadToFile(guard.name(), encryptedPayload);
		AppendToFile(guard.name(), sizeof(test::HarvestRequestEncryptedPayload) - 1);

		// Act + Assert:
		UnlockedAccountsStorage storage(guard.name());
		EXPECT_THROW(storage.load(keyPair, [](const auto&) {}), catapult_file_io_error);
	}

	TEST(TEST_CLASS, LoadThrowsWhenDataIsInvalid) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		auto keyPair = test::GenerateKeyPair();
		auto invalidPrivateBuffer = test::GenerateRandomArray<Key::Size + 1>();
		auto encryptedPayload = test::PrepareHarvestRequestEncryptedPayload(keyPair.publicKey(), invalidPrivateBuffer);
		AppendEncryptedPayloadToFile(guard.name(), encryptedPayload);

		// Act + Assert:
		UnlockedAccountsStorage storage(guard.name());
		EXPECT_THROW(storage.load(keyPair, [](const auto&) {}), catapult_runtime_error);
	}

	// endregion

	// region integration

	TEST(TEST_CLASS, CanRollbackSingleHarvestersFile) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());

		// - execute (add and save)
		auto encryptedPayloads = SeedEncryptedPayloads(storage, guard.name(), 1);
		storage.save([](const auto&) { return true; });

		// Sanity:
		test::AssertHarvesterFileContents(guard.name(), encryptedPayloads);

		// Act: undo (remove and save)
		auto requestIdentifier = test::GetRequestIdentifier(*encryptedPayloads.cbegin());
		storage.remove(requestIdentifier);
		storage.save([](const auto&) { return true; });

		// Assert: file does not exist
		EXPECT_FALSE(storage.contains(requestIdentifier));
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));
	}

	TEST(TEST_CLASS, CanRollbackMultipleHarvestersFiles) {
		// Arrange:
		test::TempFileGuard guard(Filename);
		UnlockedAccountsStorage storage(guard.name());

		// - execute (add and save, remove and save, add and save)
		auto encryptedPayloads1 = SeedEncryptedPayloads(storage, guard.name(), 1);
		storage.save([](const auto&) { return true; });

		auto requestIdentifier1 = test::GetRequestIdentifier(*encryptedPayloads1.cbegin());
		storage.remove(requestIdentifier1);
		storage.save([](const auto&) { return true; });

		auto encryptedPayloads2 = SeedEncryptedPayloads(storage, guard.name(), 1);
		storage.save([](const auto&) { return true; });

		// Sanity:
		test::AssertHarvesterFileContents(guard.name(), encryptedPayloads2);

		// Act: undo (remove and remove and save)
		auto requestIdentifier2 = test::GetRequestIdentifier(*encryptedPayloads2.cbegin());
		storage.remove(requestIdentifier2);
		storage.remove(requestIdentifier1);
		storage.save([](const auto&) { return true; });

		// Assert: file does not exist
		EXPECT_FALSE(storage.contains(requestIdentifier1));
		EXPECT_FALSE(storage.contains(requestIdentifier2));
		EXPECT_FALSE(boost::filesystem::exists(guard.name()));
	}

	// endregion
}}
