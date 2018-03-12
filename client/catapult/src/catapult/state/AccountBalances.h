#pragma once
#include "CompactMosaicUnorderedMap.h"
#include "catapult/utils/Hashers.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"
#include <unordered_map>

namespace catapult { namespace state {

	/// Container holding information about account.
	class AccountBalances {
	public:
		/// Creates an empty account balances.
		AccountBalances();

		/// Copy constructor that makes a deep copy of \a accountBalances.
		AccountBalances(const AccountBalances& accountBalances);

		/// Move constructor that move constructs an account balances from \a accountBalances.
		AccountBalances(AccountBalances&& accountBalances);

	public:
		/// Assignment operator that makes a deep copy of \a accountBalances.
		AccountBalances& operator=(const AccountBalances& accountBalances);

		/// Move assignment operator that assigns \a accountBalances.
		AccountBalances& operator=(AccountBalances&& accountBalances);

	public:
		/// Returns the number of mosaics owned.
		size_t size() const {
			return m_balances.size();
		}

		/// Returns a const iterator to the first element of the underlying set.
		auto begin() const {
			return m_balances.begin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto end() const {
			return m_balances.end();
		}

		/// Returns amount of funds of a given mosaic (\a mosaicId).
		Amount get(MosaicId mosaicId) const;

	public:
		/// Adds \a amount funds to a given mosaic (\a mosaicId).
		AccountBalances& credit(MosaicId mosaicId, Amount amount);

		/// Subtracts \a amount funds from a given mosaic (\a mosaicId).
		AccountBalances& debit(MosaicId mosaicId, Amount amount);

	private:
		CompactMosaicUnorderedMap m_balances;
	};
}}
