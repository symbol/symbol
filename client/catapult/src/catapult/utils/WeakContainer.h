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

#pragma once
#include "SpinLock.h"
#include "src/catapult/functions.h"
#include <list>
#include <memory>

namespace catapult { namespace utils {

	/// Container of weak_ptr<T> pointing to closable items.
	template<typename T>
	class WeakContainer {
	public:
		/// Creates an empty container.
		WeakContainer() : WeakContainer([](const auto&) {})
		{}

		/// Creates an empty container with a custom close function (\a close).
		WeakContainer(const consumer<T&>& close) : m_close(close)
		{}

	public:
		/// Gets the number of items in the container and removes all previously deleted items.
		size_t size() const {
			SpinLockGuard guard(m_lock);
			const_cast<WeakContainer*>(this)->pruneInternal();
			return m_entries.size();
		}

		/// Adds \a pEntry to this container and removes all previously deleted items.
		/// Returns \c false without adding when the container has already been cleared (during shutdown).
		/// This prevents a late insert from leaving a stale weak_ptr that would keep the entry's control
		/// block - and anything its custom deleter captures - alive past shutdown.
		bool insert(const std::weak_ptr<T>& pEntry) {
			SpinLockGuard guard(m_lock);
			if (m_isClosed)
				return false;

			pruneInternal();
			m_entries.push_back(pEntry);
			return true;
		}

		/// Closes and removes all items in this container and rejects subsequent inserts.
		void clear() {
			SpinLockGuard guard(m_lock);
			m_isClosed = true;
			for (const auto& pEntry : m_entries) {
				if(auto pSharedEntry = pEntry.lock(); pSharedEntry)
					m_close(*pSharedEntry);
			}

			m_entries.clear();
		}

	private:
		void pruneInternal() {
			m_entries.remove_if([](const auto& pExistingEntry) { return !pExistingEntry.lock(); });
		}

	private:
		consumer<T&> m_close;
		std::list<std::weak_ptr<T>> m_entries;
		bool m_isClosed = false;
		mutable utils::SpinLock m_lock;
	};
}}
