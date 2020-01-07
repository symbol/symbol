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
#include "catapult/types.h"
#include <memory>

namespace catapult { namespace cache {

	/// Delegating proxy around a transactions cache modifier.
	/// \note This is returned by value by BasicTransactionsCache::modifier in order to allow it to be consistent with other
	///       modifier functions.
	template<typename TTransactionInfo, typename TTransactionsCacheModifier>
	class BasicTransactionsCacheModifierProxy : public utils::MoveOnly {
	public:
		/// Creates a transactions cache modifier around \a pModifier.
		explicit BasicTransactionsCacheModifierProxy(std::unique_ptr<TTransactionsCacheModifier>&& pModifier)
				: m_pModifier(std::move(pModifier))
		{}

	public:
		/// Gets the number of transactions in the cache.
		size_t size() const {
			return m_pModifier->size();
		}

		/// Adds the transaction info (\a transactionInfo) to the cache.
		/// Returns \c true if the transaction info was successfully added.
		bool add(const TTransactionInfo& transactionInfo) {
			return m_pModifier->add(transactionInfo);
		}

		/// Removes the transaction identified by \a hash from the cache.
		TTransactionInfo remove(const Hash256& hash) {
			return m_pModifier->remove(hash);
		}

	protected:
		/// Gets the modifier.
		TTransactionsCacheModifier& modifier() {
			return *m_pModifier;
		}

		/// Gets the (const) modifier.
		const TTransactionsCacheModifier& modifier() const {
			return *m_pModifier;
		}

	private:
		std::unique_ptr<TTransactionsCacheModifier> m_pModifier;
	};
}}
