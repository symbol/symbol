#pragma once
#include "AccountStateCacheTypes.h"
#include "ReadOnlyAccountStateCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/model/NetworkInfo.h"

namespace catapult { namespace model { struct AccountInfo; } }

namespace catapult { namespace cache {

	/// Basic delta on top of the account state cache.
	class BasicAccountStateCacheDelta : public utils::MoveOnly {
	public:
		using ReadOnlyView = ReadOnlyAccountStateCache;

	private:
		using AccountStatePointer = account_state_cache_types::PointerType;
		using ConstAccountStatePointer = account_state_cache_types::ConstPointerType;

		using AddressBasedAccountStateBaseSetDelta = account_state_cache_types::address_account_state_map::BaseSetDeltaType;
		using KeyToAddressBaseSetDelta = account_state_cache_types::key_address_map::BaseSetDeltaType;

		using AccountStates = std::unordered_set<account_state_cache_types::ConstPointerType>;

	public:
		/// Creates a delta for the network specified by \a networkIdentifier with the importance grouping (\a importanceGrouping)
		/// based on an address based state map (\a pStateByAddress) and a key to address map (\a pKeyToAddress).
		explicit BasicAccountStateCacheDelta(
				model::NetworkIdentifier networkIdentifier,
				uint64_t importanceGrouping,
				const std::shared_ptr<AddressBasedAccountStateBaseSetDelta>& pStateByAddress,
				const std::shared_ptr<KeyToAddressBaseSetDelta>& pKeyToAddress)
				: m_networkIdentifier(networkIdentifier)
				, m_importanceGrouping(importanceGrouping)
				, m_pStateByAddress(pStateByAddress)
				, m_pKeyToAddress(pKeyToAddress)
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
			return m_pStateByAddress->size();
		}

		/// If not present, adds an account to the cache at given height (\a addressHeight) using \a address.
		/// Returns an account state.
		AccountStatePointer addAccount(const Address& address, Height addressHeight);

		/// If not present, adds an account to the cache using \a publicKey.
		/// If public key has not been known earlier, its height is set to \a publicKeyHeight.
		/// Returns an account state.
		AccountStatePointer addAccount(const Key& publicKey, Height publicKeyHeight);

		/// If not present, adds an account to the cache using information in \a accountInfo.
		/// Returns an account state.
		AccountStatePointer addAccount(const model::AccountInfo& accountInfo);

		/// Searches for the given \a address in the cache.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const Address& address) const;

		/// Searches for the given \a publicKey in the cache.
		/// Returns \c true if it is found or \c false if it is not found.
		/// \note This function only searches for the public key, it does not search for the address associated with the public key.
		bool contains(const Key& publicKey) const;

		/// Returns account state for an account identified by \a publicKey.
		/// \note This function only searches for the public key, it does not search for the address associated with the public key.
		AccountStatePointer findAccount(const Address& address);

		/// Returns account state for an account identified by \a publicKey.
		/// \note This function only searches for the public key, it does not search for the address associated with the public key.
		AccountStatePointer findAccount(const Key& publicKey);

		/// Returns account state for an account identified by \a address or \c nullptr if the account was not found.
		ConstAccountStatePointer findAccount(const Address& address) const;

		/// Returns account state for an account identified by \a publicKey or \c nullptr if the account was not found.
		/// \note This function only searches for the public key, it does not search for the address associated with the public key.
		ConstAccountStatePointer findAccount(const Key& publicKey) const;

	private:
		AccountStatePointer findAccountOrNull(const Address& address);
		AccountStatePointer findAccountOrNull(const Key& publicKey);

		void remove(const Address& address, Height height);
		void remove(const Key& publicKey, Height height);

	public:
		/// If \a height matches the height at which account was added, queues removal of account's \a address
		/// information from the cache, therefore queuing complete removal of the account from the cache.
		void queueRemove(const Address& address, Height height) {
			m_queuedRemoveByAddress.emplace(height, address);
		}

		/// If \a height matches the height at which account was added, queues removal of account's \a publicKey
		/// information from the cache.
		void queueRemove(const Key& publicKey, Height height) {
			m_queuedRemoveByPublicKey.emplace(height, publicKey);
		}

		/// Commits all queued removals.
		void commitRemovals();

	public:
		/// Returns a const iterator to the first element of the underlying set.
		auto cbegin() const {
			return m_pStateByAddress->cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto cend() const {
			return m_pStateByAddress->cend();
		}

	public:
		/// Gets all account states that have pending modifications.
		/// \note Both updated and new account states are included.
		AccountStates modifiedAccountStates() const;

		/// Gets all account states that are pending removal.
		AccountStates removedAccountStates() const;

	private:
		Address getAddress(const Key& publicKey);

	private:
		const model::NetworkIdentifier m_networkIdentifier;
		const uint64_t m_importanceGrouping;

		std::shared_ptr<AddressBasedAccountStateBaseSetDelta> m_pStateByAddress;
		std::shared_ptr<KeyToAddressBaseSetDelta> m_pKeyToAddress;

		account_state_cache_types::QueuedRemovalSet<Address> m_queuedRemoveByAddress;
		account_state_cache_types::QueuedRemovalSet<Key> m_queuedRemoveByPublicKey;
	};

	/// Delta on top of the account state cache.
	class AccountStateCacheDelta : public ReadOnlyViewSupplier<BasicAccountStateCacheDelta> {
	public:
		/// Creates a delta for the network specified by \a networkIdentifier with the importance grouping (\a importanceGrouping)
		/// based on an address based state map (\a pStateByAddress) and a key to address map (\a pKeyToAddress).
		explicit AccountStateCacheDelta(
				model::NetworkIdentifier networkIdentifier,
				uint64_t importanceGrouping,
				const std::shared_ptr<account_state_cache_types::address_account_state_map::BaseSetDeltaType>& pStateByAddress,
				const std::shared_ptr<account_state_cache_types::key_address_map::BaseSetDeltaType>& pKeyToAddress)
				: ReadOnlyViewSupplier(networkIdentifier, importanceGrouping, pStateByAddress, pKeyToAddress)
		{}
	};
}}
