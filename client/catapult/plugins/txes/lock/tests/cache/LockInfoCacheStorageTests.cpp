#include "src/cache/LockInfoCacheStorage.h"
#include "src/cache/HashLockInfoCacheTypes.h"
#include "src/cache/SecretLockInfoCacheTypes.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS LockInfoCacheStorageTests

	namespace {
		constexpr size_t Lock_Info_Size = Key_Size + sizeof(MosaicId) + sizeof(Amount) + sizeof(Height) + sizeof(uint8_t);
		constexpr size_t Hash_Lock_Info_Size = Lock_Info_Size + Hash256_Size;
		constexpr size_t Secret_Lock_Info_Size = Lock_Info_Size + sizeof(uint8_t) + Hash512_Size + Address_Decoded_Size;

		template<typename TLockInfoTraits>
		auto MakePair(const typename TLockInfoTraits::ValueType& lockInfo) {
			return std::make_pair(TLockInfoTraits::ToKey(lockInfo), lockInfo);
		}

		template<typename TLockInfoTraits>
		void AssertBuffer(const std::vector<typename TLockInfoTraits::ValueType>& values, const std::vector<uint8_t>& buffer) {
			std::vector<typename TLockInfoTraits::PackedValueType> packedValues;
			for (const auto& value : values)
				packedValues.emplace_back(value);

			ASSERT_EQ(sizeof(typename TLockInfoTraits::PackedValueType) * packedValues.size(), buffer.size());
			EXPECT_TRUE(0 == std::memcmp(packedValues.data(), buffer.data(), buffer.size()));
		}

#pragma pack(push, 1)

		struct PackedLockInfo {
		public:
			explicit PackedLockInfo(const model::LockInfo& lockInfo)
					: Account(lockInfo.Account)
					, MosaicId(lockInfo.MosaicId)
					, Amount(lockInfo.Amount)
					, Height(lockInfo.Height)
					, Status(lockInfo.Status)
			{}

		public:
			Key Account;
			catapult::MosaicId MosaicId;
			catapult::Amount Amount;
			catapult::Height Height;
			model::LockStatus Status;
		};

		struct PackedHashLockInfo : public PackedLockInfo {
		public:
			explicit PackedHashLockInfo(const model::HashLockInfo& hashLockInfo)
					: PackedLockInfo(hashLockInfo)
					, Hash(hashLockInfo.Hash)
			{}

		public:
			Hash256 Hash;
		};

		struct PackedSecretLockInfo : public PackedLockInfo {
		public:
			explicit PackedSecretLockInfo(const model::SecretLockInfo& secretLockInfo)
					: PackedLockInfo(secretLockInfo)
					, HashAlgorithm(secretLockInfo.HashAlgorithm)
					, Secret(secretLockInfo.Secret)
					, Recipient(secretLockInfo.Recipient)
			{}

		public:
			model::LockHashAlgorithm HashAlgorithm;
			Hash512 Secret;
			catapult::Address Recipient;
		};

#pragma pack(pop)

		struct HashTraits : public test::BasicHashLockInfoTestTraits {
			using PackedValueType = PackedHashLockInfo;
			using StorageType = test::HashLockInfoCacheFactory::LockInfoCacheStorage;

			static size_t ValueTypeSize() {
				return Hash_Lock_Info_Size;
			}
		};

		struct SecretTraits : public test::BasicSecretLockInfoTestTraits {
			using PackedValueType = PackedSecretLockInfo;
			using StorageType = test::SecretLockInfoCacheFactory::LockInfoCacheStorage;

			static size_t ValueTypeSize() {
				return Secret_Lock_Info_Size;
			}
		};

		template<typename TLockInfoTraits>
		class TestContext {
		public:
			explicit TestContext(size_t numLocks)
					: m_stream("", m_buffer)
					, m_lockInfos(test::CreateLockInfos<TLockInfoTraits>(numLocks))
			{}

		public:
			auto& buffer() {
				return m_buffer;
			}

			auto& outputStream() {
				return m_stream;
			}

			const auto& lockInfos() const {
				return m_lockInfos;
			}

		private:
			std::vector<uint8_t> m_buffer;
			mocks::MockMemoryStream m_stream;
			std::vector<typename TLockInfoTraits::ValueType> m_lockInfos;
		};
	}

#define LOCK_TYPE_BASED_TEST(TEST_NAME) \
	template<typename TLockInfoTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Hash) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<HashTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Secret) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SecretTraits>(); } \
	template<typename TLockInfoTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region Save

	namespace {
		template<typename TTraits>
		void AssertCanSaveLockInfos(size_t numLockInfos) {
			// Arrange:
			TestContext<TTraits> context(numLockInfos);

			// Act:
			for (const auto& lockInfo : context.lockInfos())
				TTraits::StorageType::Save(MakePair<TTraits>(lockInfo), context.outputStream());

			// Assert:
			AssertBuffer<TTraits>(context.lockInfos(), context.buffer());
		}
	}

	LOCK_TYPE_BASED_TEST(CanSaveSingleLockInfo) {
		// Assert:
		AssertCanSaveLockInfos<TLockInfoTraits>(1);
	}

	LOCK_TYPE_BASED_TEST(CanSaveMultipleLockInfos) {
		// Assert:
		AssertCanSaveLockInfos<TLockInfoTraits>(10);
	}

	// endregion

	// region Load

	namespace {
		template<typename TLockInfoTraits>
		std::vector<uint8_t> CreateBuffer(const std::vector<typename TLockInfoTraits::ValueType>& lockInfos) {
			std::vector<uint8_t> buffer(lockInfos.size() * TLockInfoTraits::ValueTypeSize());

			auto* pData = buffer.data();
			for (const auto& lockInfo : lockInfos) {
				using PackedValueType = typename TLockInfoTraits::PackedValueType;
				PackedValueType packedInfo(lockInfo);
				memcpy(pData, &packedInfo, sizeof(PackedValueType));
				pData += sizeof(PackedValueType);
			}

			return buffer;
		}

		template<typename TLockInfoTraits>
		void AssertCanLoadLockInfo(size_t numLockInfos) {
			// Arrange:
			TestContext<TLockInfoTraits> context(numLockInfos);
			auto buffer = CreateBuffer<TLockInfoTraits>(context.lockInfos());
			mocks::MockMemoryStream inputStream("", buffer);

			// Act:
			typename TLockInfoTraits::CacheType cache;
			auto delta = cache.createDelta();
			for (auto i = 0u; i < context.lockInfos().size(); ++i)
				TLockInfoTraits::StorageType::Load(inputStream, *delta);

			cache.commit();

			// Assert: whole buffer has been read
			EXPECT_EQ(buffer.size(), inputStream.position());

			// Assert:
			auto view = cache.createView();
			for (const auto& lockInfo : context.lockInfos()) {
				const auto& key = TLockInfoTraits::ToKey(lockInfo);
				ASSERT_TRUE(view->contains(key));

				const auto& lockInfoFromCache = view->get(key);
				TLockInfoTraits::AssertEqual(lockInfo, lockInfoFromCache);
			}
		}
	}

	LOCK_TYPE_BASED_TEST(CanLoadSingleLockInfo) {
		// Assert:
		AssertCanLoadLockInfo<TLockInfoTraits>(1);
	}

	LOCK_TYPE_BASED_TEST(CanLoadMultipleLockInfos) {
		// Assert:
		AssertCanLoadLockInfo<TLockInfoTraits>(10);
	}

	// endregion
}}
