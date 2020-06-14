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

#include "AccountStateCacheSubCachePlugin.h"
#include "catapult/cache/SummaryAwareSubCachePluginAdapter.h"

namespace catapult { namespace cache {

	namespace {
		// region serialization utils

		void WriteAddresses(const model::AddressSet& addresses, io::OutputStream& output) {
			io::Write64(output, addresses.size());
			for (const auto& address : addresses)
				output.write(address);
		}

		void WriteAccountHistories(const AddressAccountHistoryMap& accountHistories, io::OutputStream& output) {
			io::Write64(output, accountHistories.size());
			for (const auto& accountHistoryPair : accountHistories) {
				output.write(accountHistoryPair.first);

				const auto& balanceHistory = accountHistoryPair.second.balances();
				io::Write64(output, balanceHistory.size());
				for (auto height : balanceHistory.heights()) {
					io::Write(output, height);
					io::Write(output, balanceHistory.get(height));
				}
			}
		}

		template<typename THighValueAccounts>
		void WriteHighValueAccounts(const THighValueAccounts& accounts, io::OutputStream& output) {
			WriteAddresses(accounts.addresses(), output);
			WriteAccountHistories(accounts.accountHistories(), output);
			output.flush();
		}

		model::AddressSet ReadAddresses(io::InputStream& input) {
			model::AddressSet addresses;

			auto numAddresses = io::Read64(input);
			for (auto i = 0u; i < numAddresses; ++i) {
				Address address;
				input.read(address);
				addresses.insert(address);
			}

			return addresses;
		}

		state::AccountHistory ReadAccountHistory(io::InputStream& input) {
			state::AccountHistory accountHistory;

			auto numBalances = io::Read64(input);
			for (auto i = 0u; i < numBalances; ++i) {
				auto height = io::Read<Height>(input);
				auto amount = io::Read<Amount>(input);
				accountHistory.add(height, amount);
			}

			return accountHistory;
		}

		AddressAccountHistoryMap ReadAccountHistories(io::InputStream& input) {
			AddressAccountHistoryMap accountHistories;

			auto numAccountHistories = io::Read64(input);
			for (auto i = 0u; i < numAccountHistories; ++i) {
				Address address;
				input.read(address);

				accountHistories.emplace(address, ReadAccountHistory(input));
			}

			return accountHistories;
		}

		HighValueAccounts ReadHighValueAccounts(io::InputStream& input) {
			auto addresses = ReadAddresses(input);
			auto accountHistories = ReadAccountHistories(input);
			return HighValueAccounts(std::move(addresses), std::move(accountHistories));
		}

		// endregion

		// region AccountStateCacheSummaryCacheStorage

		class AccountStateCacheSummaryCacheStorage : public SummaryCacheStorage<AccountStateCache> {
		public:
			using SummaryCacheStorage<AccountStateCache>::SummaryCacheStorage;

		public:
			void saveAll(const CatapultCacheView&, io::OutputStream&) const override {
				CATAPULT_THROW_INVALID_ARGUMENT("AccountStateCacheSummaryCacheStorage does not support saveAll");
			}

			void saveSummary(const CatapultCacheDelta& cacheDelta, io::OutputStream& output) const override {
				WriteHighValueAccounts(cacheDelta.sub<AccountStateCache>().highValueAccounts(), output);
			}

			void loadAll(io::InputStream& input, size_t) override {
				cache().init(ReadHighValueAccounts(input));
			}
		};

		// endregion

		// region AccountStateFullCacheStorage

		// custom AccountStateFullCacheStorage is needed because historical balances are computed and cannot be directly
		// calculated from AccountState since only cumulative balances are stored within

		class AccountStateFullCacheStorage : public CacheStorage {
		public:
			AccountStateFullCacheStorage(std::unique_ptr<CacheStorage>&& pStorage, AccountStateCache& cache)
					: m_pStorage(std::move(pStorage))
					, m_cache(cache)
			{}

		public:
			const std::string& name() const override {
				return m_pStorage->name();
			}

		public:
			void saveAll(const CatapultCacheView& cacheView, io::OutputStream& output) const override {
				m_pStorage->saveAll(cacheView, output);
				WriteHighValueAccounts(cacheView.sub<AccountStateCache>().highValueAccounts(), output);
			}

			void saveSummary(const CatapultCacheDelta& cacheDelta, io::OutputStream& output) const override {
				m_pStorage->saveSummary(cacheDelta, output);
			}

			void loadAll(io::InputStream& input, size_t batchSize) override {
				m_pStorage->loadAll(input, batchSize);
				m_cache.init(ReadHighValueAccounts(input));
			}

		private:
			std::unique_ptr<CacheStorage> m_pStorage;
			AccountStateCache& m_cache;
		};

		// endregion
	}

	// region AccountStateCacheSubCachePlugin

	AccountStateCacheSubCachePlugin::AccountStateCacheSubCachePlugin(
			const CacheConfiguration& config,
			const AccountStateCacheTypes::Options& options)
			: BaseType(std::make_unique<AccountStateCache>(config, options))
	{}

	std::unique_ptr<CacheStorage> AccountStateCacheSubCachePlugin::createStorage() {
		auto pStorage = BaseType::createStorage();
		if (pStorage)
			return std::make_unique<AccountStateFullCacheStorage>(std::move(pStorage), this->cache());

		return std::make_unique<AccountStateCacheSummaryCacheStorage>(this->cache());
	}

	// endregion
}}
