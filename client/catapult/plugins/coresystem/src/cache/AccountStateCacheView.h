#pragma once
#include "AccountStateCacheTypes.h"
#include "ReadOnlyAccountStateCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/SpinReaderWriterLock.h"

namespace catapult { namespace cache {

	/// Basic view on top of the account state cache.
	class BasicAccountStateCacheView : public utils::MoveOnly {
	public:
		using ReadOnlyView = ReadOnlyAccountStateCache;
		using ConstAccountStatePointer = account_state_cache_types::ConstPointerType;

	public:
		/// Creates a view for the network specified by \a networkIdentifier with the importance grouping (\a importanceGrouping)
		/// around \a stateByAddress and \a keyToAddress.
		explicit BasicAccountStateCacheView(
				model::NetworkIdentifier networkIdentifier,
				uint64_t importanceGrouping,
				const account_state_cache_types::address_account_state_map::BaseSetType& stateByAddress,
				const account_state_cache_types::key_address_map::BaseSetType& keyToAddress)
				: m_networkIdentifier(networkIdentifier)
				, m_importanceGrouping(importanceGrouping)
				, m_stateByAddress(stateByAddress)
				, m_keyToAddress(keyToAddress)
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

	public:
		/// Returns the number of elements in the cache.
		size_t size() const {
			return m_stateByAddress.size();
		}

		/// Searches for the given \a address in the cache.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const Address& address) const;

		/// Searches for the given \a publicKey in the cache.
		/// Returns \c true if it is found or \c false if it is not found.
		/// \note This function only searches for the public key, it does not search for the address associated with the public key.
		bool contains(const Key& publicKey) const;

		/// Returns account state for an account identified by \a address or \c nullptr if the account was not found.
		ConstAccountStatePointer findAccount(const Address& address) const;

		/// Returns account state for an account identified by \a publicKey or \c nullptr if the account was not found.
		/// \note This function only searches for the public key, it does not search for the address associated with the public key.
		ConstAccountStatePointer findAccount(const Key& publicKey) const;

	public:
		/// Returns a const iterator to the first element of the underlying set.
		auto cbegin() const {
			return m_stateByAddress.cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto cend() const {
			return m_stateByAddress.cend();
		}

	private:
		const model::NetworkIdentifier m_networkIdentifier;
		const uint64_t m_importanceGrouping;

		const account_state_cache_types::address_account_state_map::BaseSetType& m_stateByAddress;
		const account_state_cache_types::key_address_map::BaseSetType& m_keyToAddress;
	};

	/// View on top of the account state cache.
	class AccountStateCacheView : public ReadOnlyViewSupplier<BasicAccountStateCacheView> {
	public:
		/// Creates a view for the network specified by \a networkIdentifier with the importance grouping (\a importanceGrouping)
		/// around \a stateByAddress and \a keyToAddress.
		explicit AccountStateCacheView(
				model::NetworkIdentifier networkIdentifier,
				uint64_t importanceGrouping,
				const account_state_cache_types::address_account_state_map::BaseSetType& stateByAddress,
				const account_state_cache_types::key_address_map::BaseSetType& keyToAddress)
				: ReadOnlyViewSupplier(networkIdentifier, importanceGrouping, stateByAddress, keyToAddress)
		{}
	};
}}
