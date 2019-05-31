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
				entry.setMinApproval(static_cast<uint8_t>(mainAccountId + 23));
				entry.setMinRemoval(static_cast<uint8_t>(mainAccountId + 34));

				// add cosignatories
				for (auto i = mainAccountId; i < mainAccountId + numCosignatories; ++i)
					entry.cosignatories().insert(m_accountKeys[i]);

				// add multisig accounts
				auto firstMultisigId = mainAccountId + numCosignatories;
				for (auto i = firstMultisigId; i < firstMultisigId + numMultisigAccounts; ++i)
					entry.multisigAccounts().insert(m_accountKeys[i]);

				return entry;
			}

		private:
			std::vector<uint8_t> m_buffer;
			mocks::MockMemoryStream m_stream;
			std::vector<Key> m_accountKeys;
		};

		Key ExtractKey(const uint8_t* data) {
			Key key;
			memcpy(key.data(), data, Key_Size);
			return key;
		}

		void AssertAccountKeys(const utils::SortedKeySet& expectedKeys, const uint8_t* pData) {
			ASSERT_EQ(expectedKeys.size(), *reinterpret_cast<const uint64_t*>(pData));
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
			EXPECT_EQ(entry.minApproval(), pData[0]);
			EXPECT_EQ(entry.minRemoval(), pData[1]);
			pData += 2;

			auto accountKey = ExtractKey(pData);
			EXPECT_EQ(entry.key(), accountKey);
			pData += Key_Size;

			AssertAccountKeys(entry.cosignatories(), pData);
			pData += sizeof(uint64_t) + entry.cosignatories().size() * Key_Size;

			AssertAccountKeys(entry.multisigAccounts(), pData);
			pData += sizeof(uint64_t) + entry.multisigAccounts().size() * Key_Size;

			EXPECT_EQ(pExpectedEnd, pData);
		}
	}

	// region Save

	TEST(TEST_CLASS, CanSaveSingleEntryWithNeitherCosignersNorMultisigAccounts) {
		// Arrange:
		TestContext context;
		auto entry = context.createEntry(0, 0, 0);

		// Act:
		MultisigEntrySerializer::Save(entry, context.outputStream());

		// Assert:
		auto expectedSize = sizeof(uint8_t) * 2 + sizeof(Key) + 2 * sizeof(uint64_t);
		ASSERT_EQ(expectedSize, context.buffer().size());
		AssertEntryBuffer(entry, context.buffer().data(), expectedSize);
	}

	TEST(TEST_CLASS, CanSaveSingleEntry) {
		// Arrange:
		TestContext context;
		auto entry = context.createEntry(0, 3, 4);

		// Act:
		MultisigEntrySerializer::Save(entry, context.outputStream());

		// Assert:
		auto expectedSize = sizeof(uint8_t) * 2 + sizeof(Key) + 2 * sizeof(uint64_t) + 3 * sizeof(Key) + 4 * sizeof(Key);
		ASSERT_EQ(expectedSize, context.buffer().size());
		AssertEntryBuffer(entry, context.buffer().data(), expectedSize);
	}

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
					entry.cosignatories().insert(key);
			}

			static constexpr size_t GetKeyStartBufferOffset() {
				return 2u * sizeof(uint8_t) + Key_Size;
			}
		};

		struct MultisigAccountsTraits : public MultisigEntrySerializerOrderingTraits {
		public:
			static void AddKeys(MultisigEntry& entry, const std::vector<Key>& keys) {
				for (const auto& key : keys)
					entry.multisigAccounts().insert(key);
			}

			static constexpr size_t GetKeyStartBufferOffset() {
				return 2u * sizeof(uint8_t) + Key_Size + sizeof(uint64_t);
			}
		};
	}

#define ENTRY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Cosignatories) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CosignatoriesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MultisigAccounts) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MultisigAccountsTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ENTRY_TRAITS_BASED_TEST(SavedKeysAreOrdered) {
		// Assert:
		test::SerializerOrderingTests<TTraits>::AssertSaveOrdersEntriesByKey();
	}

	// endregion

	// region Load

	namespace {
		void CopyKeySetIntoBuffer(uint8_t*& pData, const utils::SortedKeySet& keySet) {
			auto* pKey = reinterpret_cast<Key*>(pData);
			for (const auto& key : keySet)
				*pKey++ = key;

			pData += Key_Size * keySet.size();
		}

		std::vector<uint8_t> CreateBuffer(const MultisigEntry& entry) {
			// minApproval / minRemoval / key / cosignatories / multisigAccounts
			size_t cosignatoriesSize = entry.cosignatories().size();
			size_t multisigAccountsSize = entry.multisigAccounts().size();
			size_t bufferSize = 1 + 1 + Key_Size + 2 * sizeof(uint64_t) + Key_Size * (cosignatoriesSize + multisigAccountsSize);
			std::vector<uint8_t> buffer(bufferSize);

			// - minApproval / minRemoval
			buffer[0] = entry.minApproval();
			buffer[1] = entry.minRemoval();

			// - multisig account key
			auto* pData = buffer.data() + 2;
			memcpy(pData, entry.key().data(), Key_Size);
			pData += Key_Size;

			// - cosignatories
			memcpy(pData, &cosignatoriesSize, sizeof(uint64_t));
			pData += sizeof(uint64_t);
			CopyKeySetIntoBuffer(pData, entry.cosignatories());

			// - multisig accounts
			memcpy(pData, &multisigAccountsSize, sizeof(uint64_t));
			pData += sizeof(uint64_t);
			CopyKeySetIntoBuffer(pData, entry.multisigAccounts());

			return buffer;
		}

		void AssertCanLoadSingleEntry(size_t numCosignatories, size_t numMultisigAccounts) {
			// Arrange:
			TestContext context;
			auto originalEntry = context.createEntry(0, numCosignatories, numMultisigAccounts);
			auto buffer = CreateBuffer(originalEntry);

			// Act:
			MultisigEntry result(test::GenerateRandomByteArray<Key>());
			test::RunLoadValueTest<MultisigEntrySerializer>(buffer, result);

			// Assert:
			test::AssertEqual(originalEntry, result);
		}
	}

	TEST(TEST_CLASS, CanLoadSingleEntryWithNeitherCosignatoriesNorMultisigAccounts) {
		AssertCanLoadSingleEntry(0, 0);
	}

	TEST(TEST_CLASS, CanLoadSingleEntryWithCosignatoriesButWithoutMultisigAccounts) {
		AssertCanLoadSingleEntry(5, 0);
	}

	TEST(TEST_CLASS, CanLoadSingleEntryWithoutCosignatoriesButWithMultisigAccounts) {
		AssertCanLoadSingleEntry(0, 5);
	}

	TEST(TEST_CLASS, CanLoadSingleEntryWithCosignatoriesAndMultisigAccounts) {
		AssertCanLoadSingleEntry(3, 4);
	}

	// endregion
}}
