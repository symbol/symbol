#include "AccountBalances.h"

namespace catapult { namespace state {

	namespace {
		constexpr static bool IsZero(Amount amount) {
			return Amount(0) == amount;
		}
	}

	AccountBalances::AccountBalances() = default;

	AccountBalances::AccountBalances(const AccountBalances& accountBalances) {
		*this = accountBalances;
	}

	AccountBalances::AccountBalances(AccountBalances&& accountBalances) = default;

	AccountBalances& AccountBalances::operator=(const AccountBalances& accountBalances) {
		for (const auto& pair : accountBalances)
			m_balances.insert(pair);

		return *this;
	}

	AccountBalances& AccountBalances::operator=(AccountBalances&& accountBalances) = default;

	Amount AccountBalances::get(MosaicId mosaicId) const {
		auto iter = m_balances.find(mosaicId);
		return m_balances.end() == iter ? Amount(0) : iter->second;
	}

	AccountBalances& AccountBalances::credit(MosaicId mosaicId, Amount amount) {
		if (IsZero(amount))
			return *this;

		auto iter = m_balances.find(mosaicId);
		if (m_balances.end() == iter)
			m_balances.insert(std::make_pair(mosaicId, amount));
		else
			iter->second = iter->second + amount;

		return *this;
	}

	AccountBalances& AccountBalances::debit(MosaicId mosaicId, Amount amount) {
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

		iter->second = iter->second - amount;
		if (IsZero(iter->second))
			m_balances.erase(mosaicId);

		return *this;
	}
}}
