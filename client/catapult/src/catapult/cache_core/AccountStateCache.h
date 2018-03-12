#pragma once
#include "AccountStateCacheDelta.h"
#include "AccountStateCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	using AccountStateBasicCache = BasicCache<
		AccountStateCacheDescriptor,
		AccountStateCacheTypes::BaseSetType,
		AccountStateCacheTypes::Options,
		const model::AddressSet&>;

	/// Cache composed of stateful account information.
	class BasicAccountStateCache : public AccountStateBasicCache {
	public:
		/// Creates a cache around \a options.
		explicit BasicAccountStateCache(const AccountStateCacheTypes::Options& options)
				: BasicAccountStateCache(options, std::make_unique<model::AddressSet>())
		{}

	private:
		BasicAccountStateCache(const AccountStateCacheTypes::Options& options, std::unique_ptr<model::AddressSet>&& pHighValueAddresses)
				: AccountStateBasicCache(AccountStateCacheTypes::Options(options), *pHighValueAddresses)
				, m_pHighValueAddresses(std::move(pHighValueAddresses))
		{}

	public:
		/// Commits all pending changes to the underlying storage.
		/// \note This hides AccountStateBasicCache::commit.
		void commit(const CacheDeltaType& delta) {
			// high value addresses need to be captured before committing because committing clears the deltas
			auto highValueAddresses = delta.highValueAddresses();
			AccountStateBasicCache::commit(delta);
			*m_pHighValueAddresses = std::move(highValueAddresses);
		}

	private:
		// unique pointer to allow set reference to be valid after moves of this cache
		std::unique_ptr<model::AddressSet> m_pHighValueAddresses;
	};

	/// Synchronized cache composed of stateful account information.
	class AccountStateCache : public SynchronizedCache<BasicAccountStateCache> {
	public:
		DEFINE_CACHE_CONSTANTS(AccountState)

	public:
		/// Creates a cache around \a options.
		AccountStateCache(const AccountStateCacheTypes::Options& options)
				: SynchronizedCache<BasicAccountStateCache>(BasicAccountStateCache(options))
				, m_networkIdentifier(options.NetworkIdentifier)
				, m_importanceGrouping(options.ImportanceGrouping)

		{}

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const {
			return m_networkIdentifier;
		}

		/// Gets the network importance grouping.
		uint64_t importanceGrouping() const {
			return m_importanceGrouping;
		}

	private:
		model::NetworkIdentifier m_networkIdentifier;
		uint64_t m_importanceGrouping;
	};
}}
