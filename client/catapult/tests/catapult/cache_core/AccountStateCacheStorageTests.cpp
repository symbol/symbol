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

	namespace {
		void AssertCanLoadValueWithMosaics(size_t numMosaics) {
			// Arrange:
			AccountStateCache cache(Default_Cache_Options);
			auto delta = cache.createDelta();

			// - create a random account info
			auto pOriginalAccountState = std::make_unique<state::AccountState>(test::GenerateRandomAddress(), Height(123));
			test::RandomFillAccountData(0, *pOriginalAccountState, numMosaics);
			auto pOriginalAccountInfo = state::ToAccountInfo(*pOriginalAccountState);

			std::vector<uint8_t> buffer(pOriginalAccountInfo->Size);
			memcpy(buffer.data(), pOriginalAccountInfo.get(), buffer.size());
			mocks::MockMemoryStream stream("", buffer);

			// Act:
			std::vector<uint8_t> state;
			AccountStateCacheStorage::Load(stream, *delta, state);

			// Assert: the account state was loaded into the delta
			EXPECT_EQ(1u, delta->size());
			const auto* pLoadedAccountState = delta->tryGet(pOriginalAccountInfo->Address);
			ASSERT_TRUE(!!pLoadedAccountState);

			EXPECT_EQ(numMosaics, pLoadedAccountState->Balances.size());
			test::AssertEqual(*pOriginalAccountState, *pLoadedAccountState);

			// - the state buffer was resized
			EXPECT_EQ(buffer.size(), state.size());
		}
	}

	TEST(TEST_CLASS, CanLoadValue) {
		// Assert:
		AssertCanLoadValueWithMosaics(3);
	}

	TEST(TEST_CLASS, CanLoadValueWithManyMosaics) {
		// Assert:
		AssertCanLoadValueWithMosaics(65535);
	}

	TEST(TEST_CLASS, CannotLoadWhenAccountInfoSizeIsTooLarge) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();

		// - create an account info with a size one greater than max
		auto pOriginalAccountState = std::make_unique<state::AccountState>(test::GenerateRandomAddress(), Height(123));
		test::RandomFillAccountData(0, *pOriginalAccountState, 65535);
		auto pOriginalAccountInfo = state::ToAccountInfo(*pOriginalAccountState);
		pOriginalAccountInfo->Size += 1;

		std::vector<uint8_t> buffer(pOriginalAccountInfo->Size);
		memcpy(buffer.data(), pOriginalAccountInfo.get(), buffer.size() - 1); // copy the account info but not the extra byte
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		std::vector<uint8_t> state;
		EXPECT_THROW(AccountStateCacheStorage::Load(stream, *delta, state), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotLoadAccountInfoExtendingPastEndOfStream) {
		// Arrange:
		AccountStateCache cache(Default_Cache_Options);
		auto delta = cache.createDelta();

		// - create a random account info
		auto pOriginalAccountState = std::make_unique<state::AccountState>(test::GenerateRandomAddress(), Height(123));
		test::RandomFillAccountData(0, *pOriginalAccountState, 2);
		auto pOriginalAccountInfo = state::ToAccountInfo(*pOriginalAccountState);

		// - size the buffer one byte too small
		std::vector<uint8_t> buffer(pOriginalAccountInfo->Size - 1);
		memcpy(buffer.data(), pOriginalAccountInfo.get(), buffer.size());
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		std::vector<uint8_t> state;
		EXPECT_THROW(AccountStateCacheStorage::Load(stream, *delta, state), catapult_runtime_error);
	}
}}
