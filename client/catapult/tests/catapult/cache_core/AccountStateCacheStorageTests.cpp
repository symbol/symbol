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

#include "catapult/cache_core/AccountStateCacheStorage.h"
#include "catapult/state/AccountStateAdapter.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountStateCacheStorageTests

	namespace {
		constexpr auto Default_Cache_Options = AccountStateCacheTypes::Options{
			model::NetworkIdentifier::Mijin_Test,
			543,
			Amount(std::numeric_limits<Amount::ValueType>::max())
		};
	}

	// region Save

	namespace {
		void AssertCanSaveValueWithMosaics(size_t mosaicsCount) {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream("", buffer);

			// - create a random account state
			auto pOriginalAccountState = std::make_shared<state::AccountState>(test::GenerateRandomAddress(), Height(123));
			test::RandomFillAccountData(0, *pOriginalAccountState, mosaicsCount);

			// Act:
			AccountStateCacheStorage::Save(std::make_pair(Address(), pOriginalAccountState), stream);

			// Assert:
			ASSERT_EQ(sizeof(model::AccountInfo) + mosaicsCount * sizeof(model::Mosaic), buffer.size());

			const auto& savedAccountInfo = reinterpret_cast<const model::AccountInfo&>(*buffer.data());
			EXPECT_EQ(mosaicsCount, savedAccountInfo.MosaicsCount);

			auto savedAccountState = state::ToAccountState(savedAccountInfo);
			test::AssertEqual(*pOriginalAccountState, savedAccountState);

			EXPECT_EQ(0u, stream.numFlushes());
		}
	}

	TEST(TEST_CLASS, CanSaveValue) {
		// Assert:
		AssertCanSaveValueWithMosaics(3);
	}

	TEST(TEST_CLASS, CanSaveValueWithManyMosaics) {
		// Assert:
		AssertCanSaveValueWithMosaics(65535);
	}

	// endregion

	// region Load

	namespace {
		struct LoadTraits {
			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				EXPECT_THROW(AccountStateCacheStorage::Load(inputStream), catapult_runtime_error);
			}

			static void LoadAndAssert(std::vector<uint8_t>& buffer, size_t numMosaics, const state::AccountState& serializedAccountState) {
				// Arrange:
				mocks::MockMemoryStream stream("", buffer);

				// Act: load the account info and convert it to an account state for easier comparison
				auto pResult = AccountStateCacheStorage::Load(stream);
				auto loadedAccountState = state::ToAccountState(*pResult);

				// Assert:
				EXPECT_EQ(numMosaics, loadedAccountState.Balances.size());
				test::AssertEqual(serializedAccountState, loadedAccountState);
			}
		};

		struct LoadIntoTraits {
			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
				auto delta = cache.createDelta();

				std::vector<uint8_t> state;
				EXPECT_THROW(AccountStateCacheStorage::LoadInto(inputStream, *delta, state), catapult_runtime_error);
			}

			static void LoadAndAssert(std::vector<uint8_t>& buffer, size_t numMosaics, const state::AccountState& serializedAccountState) {
				// Arrange:
				mocks::MockMemoryStream stream("", buffer);

				// Act:
				AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
				auto delta = cache.createDelta();
				std::vector<uint8_t> state;
				AccountStateCacheStorage::LoadInto(stream, *delta, state);
				cache.commit();

				// Assert: the cache contains the account state
				auto view = cache.createView();
				EXPECT_EQ(1u, view->size());
				ASSERT_TRUE(view->contains(serializedAccountState.Address));
				const auto& loadedAccountState = view->get(serializedAccountState.Address);

				// - the account state contents are correct
				EXPECT_EQ(numMosaics, loadedAccountState.Balances.size());
				test::AssertEqual(serializedAccountState, loadedAccountState);

				// - the state buffer was resized
				EXPECT_EQ(buffer.size(), state.size());
			}
		};

		template<typename TLoadTraits>
		void AssertCanLoadValueWithMosaics(size_t numMosaics) {
			// Arrange: create a random account info
			auto pOriginalAccountState = std::make_unique<state::AccountState>(test::GenerateRandomAddress(), Height(123));
			test::RandomFillAccountData(0, *pOriginalAccountState, numMosaics);
			auto pOriginalAccountInfo = state::ToAccountInfo(*pOriginalAccountState);

			std::vector<uint8_t> buffer(pOriginalAccountInfo->Size);
			memcpy(buffer.data(), pOriginalAccountInfo.get(), buffer.size());

			// Act + Assert:
			TLoadTraits::LoadAndAssert(buffer, numMosaics, *pOriginalAccountState);
		}
	}

#define LOAD_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Load) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LoadTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_LoadInto) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LoadIntoTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	LOAD_TEST(CanLoadValue) {
		// Assert:
		AssertCanLoadValueWithMosaics<TTraits>(3);
	}

	LOAD_TEST(CanLoadValueWithManyMosaics) {
		// Assert:
		AssertCanLoadValueWithMosaics<TTraits>(65535);
	}

	LOAD_TEST(CannotLoadWhenAccountInfoSizeIsTooLarge) {
		// Arrange: create an account info with a size one greater than max
		auto pOriginalAccountState = std::make_unique<state::AccountState>(test::GenerateRandomAddress(), Height(123));
		test::RandomFillAccountData(0, *pOriginalAccountState, 65535);
		auto pOriginalAccountInfo = state::ToAccountInfo(*pOriginalAccountState);
		pOriginalAccountInfo->Size += 1;

		std::vector<uint8_t> buffer(pOriginalAccountInfo->Size);
		memcpy(buffer.data(), pOriginalAccountInfo.get(), buffer.size() - 1); // copy the account info but not the extra byte
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		TTraits::AssertCannotLoad(stream);
	}

	LOAD_TEST(CannotLoadAccountInfoExtendingPastEndOfStream) {
		// Arrange: create a random account info
		auto pOriginalAccountState = std::make_unique<state::AccountState>(test::GenerateRandomAddress(), Height(123));
		test::RandomFillAccountData(0, *pOriginalAccountState, 2);
		auto pOriginalAccountInfo = state::ToAccountInfo(*pOriginalAccountState);

		// - size the buffer one byte too small
		std::vector<uint8_t> buffer(pOriginalAccountInfo->Size - 1);
		memcpy(buffer.data(), pOriginalAccountInfo.get(), buffer.size());
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		TTraits::AssertCannotLoad(stream);
	}

	// endregion
}}
