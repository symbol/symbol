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

#include "src/cache/LockInfoCacheStorage.h"
#include "src/cache/HashLockInfoCacheTypes.h"
#include "src/cache/SecretLockInfoCacheTypes.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
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

		// region PackedLockInfo

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

		// endregion

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
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream("", buffer);
			auto lockInfos = test::CreateLockInfos<TTraits>(numLockInfos);

			// Act:
			for (const auto& lockInfo : lockInfos)
				TTraits::StorageType::Save(MakePair<TTraits>(lockInfo), outputStream);

			// Assert:
			AssertBuffer<TTraits>(lockInfos, buffer);
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
		struct LockInfoCacheStorageTraits {
			using KeyType = typename TLockInfoTraits::KeyType;
			using ValueType = typename TLockInfoTraits::ValueType;

			using StorageType = typename TLockInfoTraits::StorageType;
			class CacheType : public TLockInfoTraits::CacheType {
			public:
				CacheType() : TLockInfoTraits::CacheType(CacheConfiguration())
				{}
			};
		};

		template<typename TLockInfoTraits>
		using LookupCacheStorageTests = test::LookupCacheStorageTests<LockInfoCacheStorageTraits<TLockInfoTraits>>;

		template<typename TLockInfoTraits>
		struct LoadTraits {
			static constexpr auto RunLoadValueTest = LookupCacheStorageTests<TLockInfoTraits>::RunLoadValueViaLoadTest;
		};

		template<typename TLockInfoTraits>
		struct LoadIntoTraits {
			static constexpr auto RunLoadValueTest = LookupCacheStorageTests<TLockInfoTraits>::RunLoadValueViaLoadIntoTest;
		};
	}

#define LOAD_TEST_ENTRY(TEST_NAME, POSTFIX, TRAITS) \
	TEST(TEST_CLASS, TEST_NAME##_Load##POSTFIX) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TRAITS, LoadTraits<TRAITS>>(); } \
	TEST(TEST_CLASS, TEST_NAME##_LoadInto##POSTFIX) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TRAITS, LoadIntoTraits<TRAITS>>(); }

#define LOAD_TEST(TEST_NAME) \
	template<typename TLockInfoTraits, typename TLoadTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	LOAD_TEST_ENTRY(TEST_NAME, _Hash, HashTraits) \
	LOAD_TEST_ENTRY(TEST_NAME, _Secret, SecretTraits) \
	template<typename TLockInfoTraits, typename TLoadTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	LOAD_TEST(CanLoadSingleLockInfo) {
		// Arrange:
		auto originalLockInfo = test::CreateLockInfos<TLockInfoTraits>(1)[0];
		auto buffer = CreateBuffer<TLockInfoTraits>({ originalLockInfo });

		// Act:
		typename TLockInfoTraits::ValueType result;
		TLoadTraits::RunLoadValueTest(TLockInfoTraits::ToKey(originalLockInfo), buffer, result);

		// Assert:
		TLockInfoTraits::AssertEqual(originalLockInfo, result);
	}

	// endregion
}}
