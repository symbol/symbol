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

#include "src/state/MultisigEntrySerializer.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/SerializerOrderingTests.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MultisigEntrySerializerTests

	namespace {
		// region test context

		class TestContext {
		public:
			explicit TestContext(size_t numAccounts = 10)
					: m_stream(m_buffer)
					, m_accountKeys(test::GenerateKeys(numAccounts))
			{}

		public:
			auto& buffer() {
				return m_buffer;
			}

			auto& outputStream() {
				return m_stream;
			}

		public:
			auto createEntry(size_t mainAccountId, size_t numCosignatories, size_t numMultisigAccounts) {
				MultisigEntry entry(m_accountKeys[mainAccountId]);
				entry.setMinApproval(0x80000000 | static_cast<uint32_t>(mainAccountId + 23));
				entry.setMinRemoval(0x00010000 | static_cast<uint32_t>(mainAccountId + 34));

				// add cosignatories
				for (auto i = mainAccountId; i < mainAccountId + numCosignatories; ++i)
					entry.cosignatoryPublicKeys().insert(m_accountKeys[i]);

				// add multisig accounts
				auto firstMultisigId = mainAccountId + numCosignatories;
				for (auto i = firstMultisigId; i < firstMultisigId + numMultisigAccounts; ++i)
					entry.multisigPublicKeys().insert(m_accountKeys[i]);

				return entry;
			}

		private:
			std::vector<uint8_t> m_buffer;
			mocks::MockMemoryStream m_stream;
			std::vector<Key> m_accountKeys;
		};

		// endregion

		// region test utils

		Key ExtractKey(const uint8_t* data) {
			Key key;
			memcpy(key.data(), data, Key::Size);
			return key;
		}

		void AssertAccountKeys(const utils::SortedKeySet& expectedKeys, const uint8_t* pData) {
			// pData is not 8-byte aligned, so need to use memcpy
			uint64_t numKeys;
			std::memcpy(&numKeys, pData, sizeof(uint64_t));
			pData += sizeof(uint64_t);

			std::set<Key> keys;
			for (auto i = 0u; i < expectedKeys.size(); ++i) {
				auto key = ExtractKey(pData);
				pData += key.size();
				keys.insert(key);
			}

			EXPECT_EQ(expectedKeys.size(), keys.size());
			for (const auto& expectedKey : expectedKeys)
				EXPECT_CONTAINS(keys, expectedKey);
		}

		void AssertEntryBuffer(const MultisigEntry& entry, const uint8_t* pData, size_t expectedSize) {
			const auto* pExpectedEnd = pData + expectedSize;
			EXPECT_EQ(entry.minApproval(), reinterpret_cast<const uint32_t&>(pData[0]));
			EXPECT_EQ(entry.minRemoval(), reinterpret_cast<const uint32_t&>(pData[sizeof(uint32_t)]));
			pData += 2 * sizeof(uint32_t);

			auto accountKey = ExtractKey(pData);
			EXPECT_EQ(entry.key(), accountKey);
			pData += Key::Size;

			AssertAccountKeys(entry.cosignatoryPublicKeys(), pData);
			pData += sizeof(uint64_t) + entry.cosignatoryPublicKeys().size() * Key::Size;

			AssertAccountKeys(entry.multisigPublicKeys(), pData);
			pData += sizeof(uint64_t) + entry.multisigPublicKeys().size() * Key::Size;

			EXPECT_EQ(pExpectedEnd, pData);
		}

		// endregion
	}

	// region Save

	TEST(TEST_CLASS, CanSaveSingleEntryWithNeitherCosignatoriesNorMultisigAccounts) {
		// Arrange:
		TestContext context;
		auto entry = context.createEntry(0, 0, 0);

		// Act:
		MultisigEntrySerializer::Save(entry, context.outputStream());

		// Assert:
		auto expectedSize = sizeof(uint32_t) * 2 + sizeof(Key) + 2 * sizeof(uint64_t);
		ASSERT_EQ(expectedSize, context.buffer().size());
		AssertEntryBuffer(entry, context.buffer().data(), expectedSize);
	}

	TEST(TEST_CLASS, CanSaveSingleEntryWithBothCosignatoriesAndMultisigAccounts) {
		// Arrange:
		TestContext context;
		auto entry = context.createEntry(0, 3, 4);

		// Act:
		MultisigEntrySerializer::Save(entry, context.outputStream());

		// Assert:
		auto expectedSize = sizeof(uint32_t) * 2 + sizeof(Key) + 2 * sizeof(uint64_t) + 3 * sizeof(Key) + 4 * sizeof(Key);
		ASSERT_EQ(expectedSize, context.buffer().size());
		AssertEntryBuffer(entry, context.buffer().data(), expectedSize);
	}

	// endregion

	// region Save - Ordering

	namespace {
		struct MultisigEntrySerializerOrderingTraits {
		public:
			using KeyType = Key;
			using SerializerType = MultisigEntrySerializer;

			static auto CreateEntry() {
				return MultisigEntry(test::GenerateRandomByteArray<Key>());
			}
		};

		struct CosignatoriesTraits : public MultisigEntrySerializerOrderingTraits {
		public:
			static void AddKeys(MultisigEntry& entry, const std::vector<Key>& keys) {
				for (const auto& key : keys)
					entry.cosignatoryPublicKeys().insert(key);
			}

			static constexpr size_t GetKeyStartBufferOffset() {
				return 2u * sizeof(uint32_t) + Key::Size;
			}
		};

		struct MultisigAccountsTraits : public MultisigEntrySerializerOrderingTraits {
		public:
			static void AddKeys(MultisigEntry& entry, const std::vector<Key>& keys) {
				for (const auto& key : keys)
					entry.multisigPublicKeys().insert(key);
			}

			static constexpr size_t GetKeyStartBufferOffset() {
				return 2u * sizeof(uint32_t) + Key::Size + sizeof(uint64_t);
			}
		};
	}

#define ENTRY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Cosignatories) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CosignatoriesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MultisigAccounts) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MultisigAccountsTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ENTRY_TRAITS_BASED_TEST(SavedKeysAreOrdered) {
		test::SerializerOrderingTests<TTraits>::AssertSaveOrdersEntriesByKey();
	}

	// endregion

	// region Roundtrip

	namespace {
		void AssertCanRoundtripSingleEntry(size_t numCosignatories, size_t numMultisigAccounts) {
			// Arrange:
			TestContext context;
			auto originalEntry = context.createEntry(0, numCosignatories, numMultisigAccounts);

			// Act:
			auto result = test::RunRoundtripBufferTest<MultisigEntrySerializer>(originalEntry);

			// Assert:
			test::AssertEqual(originalEntry, result);
		}
	}

	TEST(TEST_CLASS, CanRoundtripSingleEntryWithNeitherCosignatoriesNorMultisigAccounts) {
		AssertCanRoundtripSingleEntry(0, 0);
	}

	TEST(TEST_CLASS, CanRoundtripSingleEntryWithCosignatoriesButWithoutMultisigAccounts) {
		AssertCanRoundtripSingleEntry(5, 0);
	}

	TEST(TEST_CLASS, CanRoundtripSingleEntryWithoutCosignatoriesButWithMultisigAccounts) {
		AssertCanRoundtripSingleEntry(0, 5);
	}

	TEST(TEST_CLASS, CanRoundtripSingleEntryWithCosignatoriesAndMultisigAccounts) {
		AssertCanRoundtripSingleEntry(3, 4);
	}

	// endregion
}}
