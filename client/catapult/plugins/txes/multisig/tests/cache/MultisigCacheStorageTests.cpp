#include "src/cache/MultisigCacheStorage.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/mocks/MemoryStream.h"
#include "tests/TestHarness.h"

#define TEST_CLASS MultisigCacheStorageTests

namespace catapult { namespace cache {

	namespace {
		class TestContext {
		public:
			explicit TestContext(size_t numAccounts = 10)
					: m_stream("", m_buffer)
					, m_accountKeys(test::GenerateKeys(numAccounts))
			{}

		public:
			auto& outputStream() {
				return m_stream;
			}

			auto& buffer() {
				return m_buffer;
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
			mocks::MemoryStream m_stream;
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

	namespace {
		void AssertCanLoadSingleEntry(size_t numCosignatories, size_t numMultisigAccounts) {
			// Arrange:
			TestContext context;
			auto entry = context.createEntry(0, numCosignatories, numMultisigAccounts);
			MultisigCacheStorage::Save(std::make_pair(entry.key(), entry), context.outputStream());
			mocks::MemoryStream inputStream("", context.buffer());

			// Act:
			MultisigCache cache;
			auto delta = cache.createDelta();
			MultisigCacheStorage::Load(inputStream, *delta);
			cache.commit();

			// Assert: whole buffer has been read
			EXPECT_EQ(context.buffer().size(), inputStream.position());

			// Assert:
			auto view = cache.createView();
			auto iter = std::find_if(view->cbegin(), view->cend(), [&entry](const auto& pair) {
				return entry.key() == pair.second.key();
			});
			ASSERT_NE(view->cend(), iter);
			AssertEqual(entry, iter->second);
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
}}
