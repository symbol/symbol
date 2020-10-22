/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
		// region serialization utils (write)

		void WriteAddresses(const model::AddressSet& addresses, io::OutputStream& output) {
			io::Write64(output, addresses.size());
			for (const auto& address : addresses)
				output.write(address);
		}

		void WritePinnedVotingKey(io::OutputStream& output, const model::PinnedVotingKey& pinnedVotingKey) {
			std::array<uint8_t, 16> padding{};
			output.write(pinnedVotingKey.VotingKey);
			output.write(padding);
			io::Write(output, pinnedVotingKey.StartEpoch);
			io::Write(output, pinnedVotingKey.EndEpoch);
		}

		void WriteHistoryMapValue(io::OutputStream& output, Amount amount) {
			io::Write(output, amount);
		}

		void WriteHistoryMapValue(io::OutputStream& output, const Key& key) {
			output.write(key);
		}

		void WriteHistoryMapValue(io::OutputStream& output, const std::vector<model::PinnedVotingKey>& keys) {
			auto count = static_cast<uint8_t>(keys.size());
			io::Write8(output, count);

			for (const auto& key : keys)
				WritePinnedVotingKey(output, key);
		}

		template<typename TValue>
		void WriteHistoryMap(const state::HeightIndexedHistoryMap<TValue>& historyMap, io::OutputStream& output) {
			io::Write64(output, historyMap.size());
			for (auto height : historyMap.heights()) {
				io::Write(output, height);
				WriteHistoryMapValue(output, historyMap.get(height));
			}
		}

		void WriteAccountHistories(const AddressAccountHistoryMap& accountHistories, io::OutputStream& output) {
			io::Write64(output, accountHistories.size());
			for (const auto& accountHistoryPair : accountHistories) {
				output.write(accountHistoryPair.first);

				WriteHistoryMap(accountHistoryPair.second.balance(), output);
				WriteHistoryMap(accountHistoryPair.second.vrfPublicKey(), output);
				WriteHistoryMap(accountHistoryPair.second.votingPublicKeys(), output);
			}
		}

		template<typename THighValueAccounts>
		void WriteHighValueAccounts(const THighValueAccounts& accounts, io::OutputStream& output) {
			WriteAddresses(accounts.addresses(), output);
			WriteAccountHistories(accounts.accountHistories(), output);
			output.flush();
		}

		// endregion

		// region serialization utils (read)

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

		void ReadPinnedVotingKey(io::InputStream& input, model::PinnedVotingKey& key) {
			std::array<uint8_t, 16> padding;
			input.read(key.VotingKey);
			input.read(padding);
			io::Read(input, key.StartEpoch);
			io::Read(input, key.EndEpoch);
		}

		void ReadHistoryMapValue(io::InputStream& input, Amount& amount) {
			amount = io::Read<Amount>(input);
		}

		void ReadHistoryMapValue(io::InputStream& input, Key& key) {
			input.read(key);
		}

		void ReadHistoryMapValue(io::InputStream& input, std::vector<model::PinnedVotingKey>& keys) {
			auto count = io::Read8(input);

			keys.resize(count);
			for (auto& key : keys)
				ReadPinnedVotingKey(input, key);
		}

		template<typename TValue>
		void ReadHistoryMap(io::InputStream& input, state::AccountHistory& accountHistory) {
			auto numValues = io::Read64(input);
			for (auto i = 0u; i < numValues; ++i) {
				auto height = io::Read<Height>(input);

				TValue value;
				ReadHistoryMapValue(input, value);

				accountHistory.add(height, value);
			}
		}

		AddressAccountHistoryMap ReadAccountHistories(io::InputStream& input) {
			AddressAccountHistoryMap accountHistories;

			auto numAccountHistories = io::Read64(input);
			for (auto i = 0u; i < numAccountHistories; ++i) {
				Address address;
				input.read(address);

				state::AccountHistory accountHistory;
				ReadHistoryMap<Amount>(input, accountHistory);
				ReadHistoryMap<Key>(input, accountHistory);
				ReadHistoryMap<std::vector<model::PinnedVotingKey>>(input, accountHistory);
				accountHistories.emplace(address, accountHistory);
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
