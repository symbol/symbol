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

#include "src/cache/MultisigCacheStorage.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MultisigCacheStorageTests

	namespace {
		class TestContext {
		public:
			explicit TestContext(size_t numAccounts = 10)
					: m_stream("", m_buffer)
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
				state::MultisigEntry entry(m_accountKeys[mainAccountId]);
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

		void AssertAccountKeys(const utils::KeySet& expectedKeys, const uint8_t* pData) {
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
				EXPECT_NE(keys.cend(), keys.find(expectedKey)) << utils::HexFormat(expectedKey);
		}

		void AssertEntryBuffer(const state::MultisigEntry& entry, const uint8_t* pData, size_t expectedSize) {
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

		void AssertEqual(const utils::KeySet& expectedAccountKeys, const utils::KeySet& accountKeys) {
			ASSERT_EQ(expectedAccountKeys.size(), accountKeys.size());
			EXPECT_EQ(expectedAccountKeys, accountKeys);
		}

		void AssertEqual(const state::MultisigEntry& expectedEntry, const state::MultisigEntry& entry) {
			EXPECT_EQ(expectedEntry.minApproval(), entry.minApproval());
			EXPECT_EQ(expectedEntry.minRemoval(), entry.minRemoval());

			EXPECT_EQ(expectedEntry.key(), entry.key());

			AssertEqual(expectedEntry.cosignatories(), entry.cosignatories());
			AssertEqual(expectedEntry.multisigAccounts(), entry.multisigAccounts());
		}
	}

	// region Save

	TEST(TEST_CLASS, CanSaveSingleEntryWithNeitherCosignersNorMultisigAccounts) {
		// Arrange:
		TestContext context;
		auto entry = context.createEntry(0, 0, 0);

		// Act:
		MultisigCacheStorage::Save(std::make_pair(entry.key(), entry), context.outputStream());

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
		MultisigCacheStorage::Save(std::make_pair(entry.key(), entry), context.outputStream());

		// Assert:
		auto expectedSize = sizeof(uint8_t) * 2 + sizeof(Key) + 2 * sizeof(uint64_t) + 3 * sizeof(Key) + 4 * sizeof(Key);
		ASSERT_EQ(expectedSize, context.buffer().size());
		AssertEntryBuffer(entry, context.buffer().data(), expectedSize);
	}

	TEST(TEST_CLASS, CanSaveMultipleEntries) {
		// Arrange:
		TestContext context(20);
		auto entry1 = context.createEntry(0, 3, 4);
		auto entry2 = context.createEntry(10, 4, 5);

		// Act:
		MultisigCacheStorage::Save(std::make_pair(entry1.key(), entry1), context.outputStream());
		MultisigCacheStorage::Save(std::make_pair(entry2.key(), entry2), context.outputStream());

		// Assert:
		auto expectedSize1 = sizeof(uint8_t) * 2 + sizeof(Key) + 2 * sizeof(uint64_t) + 3 * sizeof(Key) + 4 * sizeof(Key);
		auto expectedSize2 = sizeof(uint8_t) * 2 + sizeof(Key) + 2 * sizeof(uint64_t) + 4 * sizeof(Key) + 5 * sizeof(Key);
		ASSERT_EQ(expectedSize1 + expectedSize2, context.buffer().size());
		const auto* pBuffer1 = context.buffer().data();
		const auto* pBuffer2 = pBuffer1 + expectedSize1;
		AssertEntryBuffer(entry1, pBuffer1, expectedSize1);
		AssertEntryBuffer(entry2, pBuffer2, expectedSize2);
	}

	// endregion

	// region Load

	namespace {
		void CopyKeySetIntoBuffer(uint8_t*& pData, const utils::KeySet& keySet) {
			auto* pKey = reinterpret_cast<Key*>(pData);
			for (const auto& key : keySet)
				*pKey++ = key;

			pData += Key_Size * keySet.size();
		}

		std::vector<uint8_t> CreateBuffer(const state::MultisigEntry& entry) {
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

		struct MultisigCacheStorageTraits {
			using KeyType = Key;
			using ValueType = state::MultisigEntry;

			using StorageType = MultisigCacheStorage;
			class CacheType : public MultisigCache {
			public:
				CacheType() : MultisigCache(CacheConfiguration())
				{}
			};
		};

		using LookupCacheStorageTests = test::LookupCacheStorageTests<MultisigCacheStorageTraits>;

		struct LoadTraits {
			static constexpr auto RunLoadValueTest = LookupCacheStorageTests::RunLoadValueViaLoadTest;
		};

		struct LoadIntoTraits {
			static constexpr auto RunLoadValueTest = LookupCacheStorageTests::RunLoadValueViaLoadIntoTest;
		};

		template<typename TTraits>
		void AssertCanLoadSingleEntry(size_t numCosignatories, size_t numMultisigAccounts) {
			// Arrange:
			TestContext context;
			auto originalEntry = context.createEntry(0, numCosignatories, numMultisigAccounts);
			auto buffer = CreateBuffer(originalEntry);

			// Act:
			state::MultisigEntry result(test::GenerateRandomData<Key_Size>());
			TTraits::RunLoadValueTest(originalEntry.key(), buffer, result);

			// Assert:
			AssertEqual(originalEntry, result);
		}
	}

#define LOAD_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Load) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LoadTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_LoadInto) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LoadIntoTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	LOAD_TEST(CanLoadSingleEntryWithNeitherCosignatoriesNorMultisigAccounts) {
		AssertCanLoadSingleEntry<TTraits>(0, 0);
	}

	LOAD_TEST(CanLoadSingleEntryWithCosignatoriesButWithoutMultisigAccounts) {
		AssertCanLoadSingleEntry<TTraits>(5, 0);
	}

	LOAD_TEST(CanLoadSingleEntryWithoutCosignatoriesButWithMultisigAccounts) {
		AssertCanLoadSingleEntry<TTraits>(0, 5);
	}

	LOAD_TEST(CanLoadSingleEntryWithCosignatoriesAndMultisigAccounts) {
		AssertCanLoadSingleEntry<TTraits>(3, 4);
	}

	// endregion
}}
