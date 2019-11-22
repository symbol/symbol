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

#pragma once
#include "DelegatePrioritizationPolicy.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/utils/Hashers.h"
#include <vector>

namespace catapult { namespace harvesting {

	// region UnlockedAccountsAddResult

#define UNLOCKED_ACCOUNTS_ADD_RESULT_LIST \
	/* Account was successfully (newly) unlocked. */ \
	ENUM_VALUE(Success_New) \
	\
	/* Account was successfully (previously) unlocked. */ \
	ENUM_VALUE(Success_Redundant) \
	\
	/* Account could not be unlocked because it is ineligible for harvesting. */ \
	ENUM_VALUE(Failure_Harvesting_Ineligible) \
	\
	/* Account could not be unlocked because it is blocked from harvesting. */ \
	ENUM_VALUE(Failure_Harvesting_Blocked) \
	\
	/* Account could not be unlocked because limit on the server has been hit. */ \
	ENUM_VALUE(Failure_Server_Limit)

#define ENUM_VALUE(LABEL) LABEL,
	/// Possible results of an add (unlock) operation.
	enum class UnlockedAccountsAddResult {
		UNLOCKED_ACCOUNTS_ADD_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, UnlockedAccountsAddResult value);

	// endregion

	/// Container used by unlocked accounts to store key pairs.
	using UnlockedAccountsKeyPairContainer = std::vector<std::pair<crypto::KeyPair, size_t>>;

	/// Read only view on top of unlocked accounts.
	class UnlockedAccountsView : utils::MoveOnly {
	public:
		/// Creates a view around \a prioritizedKeyPairs with lock context \a readLock.
		UnlockedAccountsView(
				const UnlockedAccountsKeyPairContainer& prioritizedKeyPairs,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Gets the number of unlocked accounts.
		size_t size() const;

		/// Returns \c true if the public key belongs to an unlocked account, \c false otherwise.
		bool contains(const Key& publicKey) const;

		/// Calls \a consumer with key pairs until all are consumed or \c false is returned by consumer.
		void forEach(const predicate<const crypto::KeyPair&>& consumer) const;

	private:
		const UnlockedAccountsKeyPairContainer& m_prioritizedKeyPairs;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// Write only view on top of unlocked accounts.
	class UnlockedAccountsModifier : utils::MoveOnly {
	private:
		using KeyPredicate = predicate<const Key&>;

	public:
		/// Creates a view around \a maxUnlockedAccounts, \a prioritizer and \a prioritizedKeyPairs with lock context \a writeLock.
		UnlockedAccountsModifier(
				size_t maxUnlockedAccounts,
				const DelegatePrioritizer& prioritizer,
				UnlockedAccountsKeyPairContainer& prioritizedKeyPairs,
				utils::SpinReaderWriterLock::WriterLockGuard&& writeLock);

	public:
		/// Adds (unlocks) the account identified by key pair (\a keyPair).
		UnlockedAccountsAddResult add(crypto::KeyPair&& keyPair);

		/// Removes (locks) the account identified by the public key (\a publicKey).
		bool remove(const Key& publicKey);

		/// Removes all accounts for which \a predicate returns \c true.
		void removeIf(const KeyPredicate& predicate);

	private:
		size_t m_maxUnlockedAccounts;
		DelegatePrioritizer m_prioritizer;
		UnlockedAccountsKeyPairContainer& m_prioritizedKeyPairs;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	/// Container of all unlocked (harvesting candidate) accounts.
	class UnlockedAccounts {
	public:
		/// Creates an unlocked accounts container that allows at most \a maxUnlockedAccounts unlocked accounts
		/// and uses \a prioritizer to look up prioritization scores.
		UnlockedAccounts(size_t maxUnlockedAccounts, const DelegatePrioritizer& prioritizer);

	public:
		/// Gets a read only view of the unlocked accounts.
		UnlockedAccountsView view() const;

		/// Gets a write only view of the unlocked accounts.
		UnlockedAccountsModifier modifier();

	private:
		size_t m_maxUnlockedAccounts;
		DelegatePrioritizer m_prioritizer;
		UnlockedAccountsKeyPairContainer m_prioritizedKeyPairs;
		mutable utils::SpinReaderWriterLock m_lock;
	};
}}
