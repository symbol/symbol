#include "src/cache/AccountStateCacheStorage.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/mocks/MemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		void AssertCanSaveValueWithMosaics(size_t numMosaics) {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MemoryStream stream("", buffer);

			// - create a random account state
			auto pOriginalAccountState = std::make_shared<state::AccountState>(test::GenerateRandomAddress(), Height(123));
			test::RandomFillAccountData(0, *pOriginalAccountState, numMosaics);

			// Act:
			AccountStateCacheStorage::Save(std::make_pair(Address(), pOriginalAccountState), stream);

			// Assert:
			ASSERT_EQ(sizeof(model::AccountInfo) + numMosaics * sizeof(model::Mosaic), buffer.size());

			const auto& savedAccountInfo = reinterpret_cast<const model::AccountInfo&>(*buffer.data());
			EXPECT_EQ(numMosaics, savedAccountInfo.NumMosaics);

			state::AccountState savedAccountState(savedAccountInfo);
			test::AssertEqual(*pOriginalAccountState, savedAccountState);

			EXPECT_EQ(0u, stream.numFlushes());
		}
	}

	TEST(AccountStateCacheStorageTests, CanSaveValue) {
		// Assert:
		AssertCanSaveValueWithMosaics(3);
	}

	TEST(AccountStateCacheStorageTests, CanSaveValueWithManyMosaics) {
		// Assert:
		AssertCanSaveValueWithMosaics(65535);
	}

	namespace {
		void AssertCanLoadValueWithMosaics(size_t numMosaics) {
			// Arrange:
			AccountStateCache cache(Network_Identifier, 543);
			auto delta = cache.createDelta();

			// - create a random account info
			auto pOriginalAccountState = std::make_unique<state::AccountState>(test::GenerateRandomAddress(), Height(123));
			test::RandomFillAccountData(0, *pOriginalAccountState, numMosaics);
			auto pOriginalAccountInfo = pOriginalAccountState->toAccountInfo();

			std::vector<uint8_t> buffer(pOriginalAccountInfo->Size);
			memcpy(buffer.data(), pOriginalAccountInfo.get(), buffer.size());
			mocks::MemoryStream stream("", buffer);

			// Act:
			std::vector<uint8_t> state;
			AccountStateCacheStorage::Load(stream, *delta, state);

			// Assert: the account state was loaded into the delta
			EXPECT_EQ(1u, delta->size());
			auto pLoadedAccountState = delta->findAccount(pOriginalAccountInfo->Address);
			ASSERT_TRUE(!!pLoadedAccountState);

			EXPECT_EQ(numMosaics, pLoadedAccountState->Balances.size());
			test::AssertEqual(*pOriginalAccountState, *pLoadedAccountState);

			// - the state buffer was resized
			EXPECT_EQ(buffer.size(), state.size());
		}
	}

	TEST(AccountStateCacheStorageTests, CanLoadValue) {
		// Assert:
		AssertCanLoadValueWithMosaics(3);
	}

	TEST(AccountStateCacheStorageTests, CanLoadValueWithManyMosaics) {
		// Assert:
		AssertCanLoadValueWithMosaics(65535);
	}

	TEST(AccountStateCacheStorageTests, CannotLoadWhenAccountInfoSizeIsTooLarge) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// - create an account info with a size one greater than max
		auto pOriginalAccountState = std::make_unique<state::AccountState>(test::GenerateRandomAddress(), Height(123));
		test::RandomFillAccountData(0, *pOriginalAccountState, 65535);
		auto pOriginalAccountInfo = pOriginalAccountState->toAccountInfo();
		pOriginalAccountInfo->Size += 1;

		std::vector<uint8_t> buffer(pOriginalAccountInfo->Size);
		memcpy(buffer.data(), pOriginalAccountInfo.get(), buffer.size());
		mocks::MemoryStream stream("", buffer);

		// Act:
		std::vector<uint8_t> state;
		EXPECT_THROW(AccountStateCacheStorage::Load(stream, *delta, state), catapult_runtime_error);
	}

	TEST(AccountStateCacheStorageTests, CannotLoadAccountInfoExtendingPastEndOfStream) {
		// Arrange:
		AccountStateCache cache(Network_Identifier, 543);
		auto delta = cache.createDelta();

		// - create a random account info
		auto pOriginalAccountState = std::make_unique<state::AccountState>(test::GenerateRandomAddress(), Height(123));
		test::RandomFillAccountData(0, *pOriginalAccountState, 2);
		auto pOriginalAccountInfo = pOriginalAccountState->toAccountInfo();

		// - size the buffer one byte too small
		std::vector<uint8_t> buffer(pOriginalAccountInfo->Size - 1);
		memcpy(buffer.data(), pOriginalAccountInfo.get(), buffer.size());
		mocks::MemoryStream stream("", buffer);

		// Act:
		std::vector<uint8_t> state;
		EXPECT_THROW(AccountStateCacheStorage::Load(stream, *delta, state), catapult_runtime_error);
	}
}}
