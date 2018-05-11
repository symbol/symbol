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

#include "src/handlers/AccountInfosProducerFactory.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/state/AccountStateAdapter.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace handlers {

#define TEST_CLASS AccountInfosProducerFactoryTests

	namespace {
		using AccountInfos = std::vector<std::shared_ptr<const model::AccountInfo>>;
		using AccountStates = std::vector<state::AccountState*>;

		auto ExtractAccountStates(const cache::AccountStateCache& cache) {
			AccountStates accountStates;
			auto view = cache.createView();
			auto pIterableView = view->tryMakeIterableView();
			for (const auto& pair : *pIterableView)
				accountStates.push_back(pair.second.get());

			return accountStates;
		}

		auto ToAddressRange(const AccountStates& accountStates, const std::vector<size_t>& indexes) {
			std::vector<Address> addresses;
			for (auto index : indexes)
				addresses.push_back(accountStates[index]->Address);

			return model::AddressRange::CopyFixed(reinterpret_cast<const uint8_t*>(addresses.data()), indexes.size());
		}

		auto ToAccountInfos(const AccountStates& accountStates, const std::vector<size_t>& indexes) {
			AccountInfos accountInfos;
			for (auto index : indexes)
				accountInfos.push_back(state::ToAccountInfo(*accountStates[index]));

			return accountInfos;
		}

		auto PrepareCache(size_t count) {
			auto pCache = std::make_unique<cache::AccountStateCache>(cache::CacheConfiguration(), cache::AccountStateCacheTypes::Options{
				model::NetworkIdentifier::Mijin_Test,
				777,
				Amount(std::numeric_limits<Amount::ValueType>::max())
			});
			auto delta = pCache->createDelta();
			for (auto i = 0u; i < count; ++i) {
				auto address = test::GenerateRandomData<Address_Decoded_Size>();
				delta->addAccount(address, Height(i + 1));
			}

			pCache->commit();
			return pCache;
		}

		void AssertEqual(const AccountInfos& expectedInfos, const AccountInfos& actualInfos) {
			// Assert:
			ASSERT_EQ(expectedInfos.size(), actualInfos.size());

			auto i = 0u;
			for (const auto& pInfo : expectedInfos) {
				EXPECT_EQ(pInfo->Address, actualInfos[i]->Address) << "address at " << i;
				EXPECT_EQ(pInfo->AddressHeight, actualInfos[i]->AddressHeight) << "height at " << i;
				++i;
			}
		}

		struct AccountInfosContext {
		public:
			explicit AccountInfosContext(size_t count)
					: pCache(PrepareCache(count))
					, States(ExtractAccountStates(*pCache))
					, ProducerFactory(CreateAccountInfosProducerFactory(*pCache))
			{}

			std::unique_ptr<cache::AccountStateCache> pCache;
			AccountStates States;
			AccountInfosProducerFactory ProducerFactory;
		};

		constexpr size_t Num_Account_States = 5;

		void AssertCanSupplyAccountInfos(const std::vector<size_t>& indexes) {
			// Arrange:
			AccountInfosContext context(Num_Account_States);
			auto addresses = ToAddressRange(context.States, indexes);
			auto expectedInfos = ToAccountInfos(context.States, indexes);

			// Act:
			auto accountInfoProducer = context.ProducerFactory(addresses);

			// Assert:
			AssertEqual(expectedInfos, test::ProduceAll(accountInfoProducer));
		}
	}

	TEST(TEST_CLASS, CanSupplySingleAccountInfo) {
		// Assert:
		for (auto i = 0u; i < Num_Account_States; ++i)
			AssertCanSupplyAccountInfos({ i });
	}

	TEST(TEST_CLASS, CanSupplyMultipleAccountInfos) {
		// Assert:
		AssertCanSupplyAccountInfos({ 0, 2, 4 });
		AssertCanSupplyAccountInfos({ 1, 3 });
	}

	TEST(TEST_CLASS, CanSupplyAllAccountInfos) {
		// Assert:
		auto indexes = std::vector<size_t>(Num_Account_States);
		std::iota(indexes.begin(), indexes.end(), 0);
		AssertCanSupplyAccountInfos(indexes);
	}

	TEST(TEST_CLASS, ReturnsMinimalInitializedAccountInfosForUnknownAccounts) {
		// Arrange: change addresses so the corresponding account state cannot be found in the cache
		AccountInfosContext context(Num_Account_States);
		for (auto i = 0u; i < Num_Account_States; ++i)
			context.States[i]->Address[0] ^= 0xFF;

		auto addresses = ToAddressRange(context.States, { 0, 1, 2, 3, 4 });

		// Act:
		auto accountInfoProducer = context.ProducerFactory(addresses);

		// Assert:
		for (auto i = 0u; i < Num_Account_States; ++i) {
			auto pAccountInfo = accountInfoProducer();
			auto pExpectedUnknownAccountInfo = model::AccountInfo::FromAddress(pAccountInfo->Address);
			EXPECT_EQ(sizeof(model::AccountInfo), pAccountInfo->Size);
			EXPECT_TRUE(0 == std::memcmp(pExpectedUnknownAccountInfo.get(), pAccountInfo.get(), pAccountInfo->Size));
		}
	}
}}
