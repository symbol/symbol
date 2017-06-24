#pragma once
#include "plugins/coresystem/src/cache/AccountStateCache.h"
#include "tests/test/plugins/ObserverTestContext.h"

namespace catapult { namespace test {

	/// Observer test context that wraps an observer context and exposes functions for interacting with the account state cache.
	class AccountObserverTestContext : public test::ObserverTestContext {
	public:
		using ObserverTestContext::ObserverTestContext;

	public:
		/// Finds the account identified by \a accountIdentifier.
		template<typename IdType>
		std::shared_ptr<const state::AccountState> find(const IdType& accountIdentifier) const {
			return cache().sub<cache::AccountStateCache>().findAccount(accountIdentifier);
		}

	private:
		std::shared_ptr<state::AccountState> addAccount(const Key& publicKey) {
			return cache().sub<cache::AccountStateCache>().addAccount(publicKey, Height(1));
		}

		std::shared_ptr<state::AccountState> addAccount(const Address& address) {
			return cache().sub<cache::AccountStateCache>().addAccount(address, Height(1234));
		}

	public:
		/// Sets the (xem) balance of the account identified by \a accountIdentifier to \a amount.
		template<typename IdType>
		state::AccountBalances& setAccountBalance(const IdType& accountIdentifier, Amount::ValueType amount) {
			return setAccountBalance(accountIdentifier, Amount(amount));
		}

		/// Sets the (xem) balance of the account identified by \a accountIdentifier to \a amount.
		template<typename IdType>
		state::AccountBalances& setAccountBalance(const IdType& accountIdentifier, Amount amount) {
			auto pAccountState = addAccount(accountIdentifier);
			pAccountState->Balances.credit(Xem_Id, amount);
			return pAccountState->Balances;
		}

		/// Gets the (xem) balance of the account identified by \a accountIdentifier.
		template<typename IdType>
		Amount getAccountBalance(const IdType& accountIdentifier) const {
			auto pAccountState = find(accountIdentifier);
			if (!pAccountState)
				CATAPULT_THROW_RUNTIME_ERROR_1("could not find account in cache", utils::HexFormat(accountIdentifier));

			return pAccountState->Balances.get(Xem_Id);
		}
	};
}}
