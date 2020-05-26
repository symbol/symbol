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

#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/utils/HexParser.h"
#include "catapult/utils/StackTimer.h"
#include "tests/int/stress/test/InputDependentTest.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS BasePatriciaTreeIntegrityTests

	// region known input / known output

	namespace {
		template<typename TSerializer>
		std::pair<state::AccountState, bool> ParseAccount(const std::vector<std::string>& parts) {
			// indexes into parsed line
			enum : size_t { Placeholder, Address, AccountState };

			auto address = model::StringToAddress(parts[Address]);

			std::vector<uint8_t> serializedBuffer(parts[AccountState].size() / 2);
			utils::ParseHexStringIntoContainer(parts[AccountState].data(), parts[AccountState].size(), serializedBuffer);
			auto accountState = TSerializer::DeserializeValue(serializedBuffer);

			return std::make_pair(accountState, address == accountState.Address);
		}

		AccountStateCacheTypes::Options CreateAccountStateCacheOptions() {
			// CurrencyId must match id used when generating resources
			return test::CreateDefaultAccountStateCacheOptions(MosaicId(0xE329'AD1C'BE7F'C60D), MosaicId(2222));
		}

		template<typename TSerializer>
		void AssertAccountStateMerkleRootIsCalculatedCorrectly(
				const std::string& sourceFilename,
				const std::string& expectedMerkleRootStr) {
			// Arrange: create a db-backed account state cache
			test::TempDirectoryGuard dbDirGuard;
			CacheConfiguration cacheConfig(dbDirGuard.name(), utils::FileSize::FromMegabytes(5), PatriciaTreeStorageMode::Enabled);
			AccountStateCache cache(cacheConfig, CreateAccountStateCacheOptions());

			// - load all test accounts into the delta
			std::pair<Hash256, bool> deltaMerkleRootPair;
			{
				auto delta = cache.createDelta();
				test::RunInputDependentTest(sourceFilename, ParseAccount<TSerializer>, [&delta](const auto& accountState) {
					delta->addAccount(accountState);
				});

				// Act: calculate the delta state hash and commit
				delta->updateMerkleRoot(Height(123));
				deltaMerkleRootPair = delta->tryGetMerkleRoot();
				cache.commit();
			}

			// Act: calculate the committed state hash from the view
			auto committedMerkleRootPair = cache.createView()->tryGetMerkleRoot();

			// Assert: merkle root should be enabled
			EXPECT_TRUE(deltaMerkleRootPair.second);
			EXPECT_TRUE(committedMerkleRootPair.second);

			// - merkle roots should be equal
			EXPECT_EQ(deltaMerkleRootPair.first, committedMerkleRootPair.first);

			// - merkle roots should have expected value
			Hash256 expectedMerkleRoot;
			utils::ParseHexStringIntoContainer(expectedMerkleRootStr.data(), expectedMerkleRootStr.size(), expectedMerkleRoot);

			EXPECT_EQ(expectedMerkleRoot, deltaMerkleRootPair.first);
			EXPECT_EQ(expectedMerkleRoot, committedMerkleRootPair.first);
		}
	}

	// both serializers yield the same merkle roots because the deserializer ignores historical importances
	// so their presence or not should have no effect

	TEST(TEST_CLASS, AccountStateCacheMerkleRootIsCalculatedCorrectly_Primary_1) {
		AssertAccountStateMerkleRootIsCalculatedCorrectly<AccountStatePrimarySerializer>(
				"../tests/int/stress/resources/1.patricia-tree-account.dat",
				"ECB538F48F2BBDE2BECF01EF0BFF00DAE26D687A193553F866AF5565531DBA09");
	}

	TEST(TEST_CLASS, AccountStateCacheMerkleRootIsCalculatedCorrectly_PatriciaTree_1) {
		AssertAccountStateMerkleRootIsCalculatedCorrectly<AccountStatePatriciaTreeSerializer>(
				"../tests/int/stress/resources/1.patricia-tree-account.dat",
				"ECB538F48F2BBDE2BECF01EF0BFF00DAE26D687A193553F866AF5565531DBA09");
	}

	TEST(TEST_CLASS, AccountStateCacheMerkleRootIsCalculatedCorrectly_Primary_2) {
		AssertAccountStateMerkleRootIsCalculatedCorrectly<AccountStatePrimarySerializer>(
				"../tests/int/stress/resources/2.patricia-tree-account.dat",
				"2120B14C5259B071CB2C5884EA51B03A5B3D55934B56F7E9D6856FE62915175F");
	}

	TEST(TEST_CLASS, AccountStateCacheMerkleRootIsCalculatedCorrectly_PatriciaTree_2) {
		AssertAccountStateMerkleRootIsCalculatedCorrectly<AccountStatePatriciaTreeSerializer>(
				"../tests/int/stress/resources/2.patricia-tree-account.dat",
				"2120B14C5259B071CB2C5884EA51B03A5B3D55934B56F7E9D6856FE62915175F");
	}

	TEST(TEST_CLASS, AccountStateCacheMerkleRootIsCalculatedCorrectly_Primary_3) {
		AssertAccountStateMerkleRootIsCalculatedCorrectly<AccountStatePrimarySerializer>(
				"../tests/int/stress/resources/3.patricia-tree-account.dat",
				"3F901A77C87BABF4FEA497F0A6A57AC58CC0647A63A7D989BE8EB664F928D1E8");
	}

	TEST(TEST_CLASS, AccountStateCacheMerkleRootIsCalculatedCorrectly_PatriciaTree_3) {
		AssertAccountStateMerkleRootIsCalculatedCorrectly<AccountStatePatriciaTreeSerializer>(
				"../tests/int/stress/resources/3.patricia-tree-account.dat",
				"3F901A77C87BABF4FEA497F0A6A57AC58CC0647A63A7D989BE8EB664F928D1E8");
	}

	// endregion

	// region stress tests

	namespace {
		size_t GetNumStressAccounts() {
			return test::GetStressIterationCount() ? 200'000 : 20'000;
		}

		template<typename TAction>
		void RunTimedStressAction(const char* description, TAction action) {
			CATAPULT_LOG(debug) << "START: " << description;

			utils::StackTimer timer;
			action();
			auto elapsedMills = timer.millis();

			auto elapsedNanosPerAccount = elapsedMills * 1000 / GetNumStressAccounts();
			CATAPULT_LOG(debug) << "  END: " << description << " - " << elapsedMills << "ms (" << elapsedNanosPerAccount << "ns avg)";
		}

		void AssertCanApplyManyAddsToTree(size_t numBatches) {
			// Arrange: create a db-backed account state cache
			CATAPULT_LOG(debug) << "creating patricia tree enabled cache";
			test::TempDirectoryGuard dbDirGuard;
			CacheConfiguration cacheConfig(dbDirGuard.name(), utils::FileSize::FromMegabytes(5), PatriciaTreeStorageMode::Enabled);
			AccountStateCache cache(cacheConfig, CreateAccountStateCacheOptions());

			// - load all test accounts into the delta
			for (auto i = 0u; i < numBatches; ++i) {
				std::vector<Address> addresses(GetNumStressAccounts() / numBatches);
				test::FillWithRandomData({ reinterpret_cast<uint8_t*>(addresses.data()), addresses.size() * sizeof(Address) });

				auto delta = cache.createDelta();
				RunTimedStressAction("adding accounts", [&delta, &addresses]() {
					for (const auto& address : addresses)
						delta->addAccount(address, Height(1));
				});

				// Act: calculate the delta state hash
				RunTimedStressAction("calculating state hash", [&delta]() {
					delta->updateMerkleRoot(Height(123));
				});

				RunTimedStressAction("committing all changes", [&cache]() {
					cache.commit();
				});
			}

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(GetNumStressAccounts(), view->size());

			auto merkleRootPair = view->tryGetMerkleRoot();
			EXPECT_TRUE(merkleRootPair.second);
			EXPECT_NE(Hash256(), merkleRootPair.first);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, Stress_CanApplyManyAddsToTree) {
		AssertCanApplyManyAddsToTree(1);
	}

	NO_STRESS_TEST(TEST_CLASS, Stress_CanApplyManyAddsToTree_MultipleBatches) {
		AssertCanApplyManyAddsToTree(10);
	}

	// endregion
}}
