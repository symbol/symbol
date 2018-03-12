#include "src/handlers/AccountInfosSupplier.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/state/AccountStateAdapter.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace handlers {

#define TEST_CLASS AccountInfosSupplierTests

	namespace {
		using AccountInfos = std::vector<std::shared_ptr<const model::AccountInfo>>;
		using AccountStates = std::vector<state::AccountState*>;

		auto ExtractAccountStates(const cache::AccountStateCache& cache) {
			AccountStates accountStates;
			auto view = cache.createView();
			for (const auto& pair : *view)
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
			auto pCache = std::make_unique<cache::AccountStateCache>(cache::AccountStateCacheTypes::Options{
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
					, Supplier(CreateAccountInfosSupplier(*pCache))
			{}

			std::unique_ptr<cache::AccountStateCache> pCache;
			AccountStates States;
			AccountInfosSupplier Supplier;
		};

		constexpr size_t Num_Account_States = 5;

		void AssertCanSupplyAccountInfos(const std::vector<size_t>& indexes) {
			// Arrange:
			AccountInfosContext context(Num_Account_States);
			auto addresses = ToAddressRange(context.States, indexes);
			auto expectedInfos = ToAccountInfos(context.States, indexes);

			// Act:
			auto accountInfos = context.Supplier(addresses);

			// Assert:
			AssertEqual(expectedInfos, accountInfos);
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
		auto accountInfos = context.Supplier(addresses);

		// Assert:
		for (auto i = 0u; i < Num_Account_States; ++i) {
			auto pAccountInfo = model::AccountInfo::FromAddress(accountInfos[i]->Address);
			EXPECT_EQ(sizeof(model::AccountInfo), accountInfos[i]->Size);
			EXPECT_TRUE(0 == std::memcmp(pAccountInfo.get(), accountInfos[i].get(), accountInfos[i]->Size));
		}
	}
}}
