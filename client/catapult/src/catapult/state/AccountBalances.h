#pragma once
#include "catapult/utils/Hashers.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"
#include <unordered_map>

namespace catapult { namespace state {

	/// Container holding information about account.
	class AccountBalances {
	public:
		/// Returns the number of mosaics owned.
		size_t size() const {
			return m_balances.size();
		}

		/// Returns amount of funds of a given mosaic (\a mosaicId).
		Amount get(MosaicId mosaicId) const {
			auto it = m_balances.find(mosaicId);
			return m_balances.cend() == it ? Amount(0) : it->second;
		}

		/// Returns a const iterator to the first element of the underlying set.
		auto begin() const {
			return m_balances.cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto end() const {
			return m_balances.cend();
		}

	public:
		/// Adds \a amount funds to a given mosaic (\a mosaicId).
		AccountBalances& credit(MosaicId mosaicId, Amount amount) {
			if (IsZero(amount))
				return *this;

			// there's a side-effect here, that balance will be set to zero
			// if it wasn't in map yet
			auto& currentBalance = m_balances[mosaicId];
			currentBalance = currentBalance + amount;
			return *this;
		}

		/// Subtracts \a amount funds from a given mosaic (\a mosaicId).
		AccountBalances& debit(MosaicId mosaicId, Amount amount) {
			if (IsZero(amount))
				return *this;

			auto iter = m_balances.find(mosaicId);
			auto hasZeroBalance = m_balances.end() == iter;
			if (hasZeroBalance || amount > iter->second) {
				CATAPULT_THROW_RUNTIME_ERROR_2(
						"debit amount is greater than current balance",
						amount,
						hasZeroBalance ? Amount(0) : iter->second);
			}

			auto& currentBalance = iter->second;
			currentBalance = currentBalance - amount;
			if (IsZero(currentBalance))
				m_balances.erase(mosaicId);

			return *this;
		}

	private:
		constexpr static bool IsZero(Amount amount) {
			return Amount(0) == amount;
		}

	private:
		std::unordered_map<MosaicId, Amount, utils::BaseValueHasher<MosaicId>> m_balances;
	};
}}
