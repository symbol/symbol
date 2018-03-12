#pragma once
#include "SpinLock.h"
#include "catapult/functions.h"
#include <list>
#include <memory>

namespace catapult { namespace utils {

	/// A container of weak_ptr<T> pointing to closable items.
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
		void insert(const std::weak_ptr<T>& pEntry) {
			SpinLockGuard guard(m_lock);
			pruneInternal();
			m_entries.push_back(pEntry);
		}

		/// Closes and removes all items in this container.
		void clear() {
			SpinLockGuard guard(m_lock);
			for (const auto& pEntry : m_entries) {
				auto pSharedEntry = pEntry.lock();
				if (!pSharedEntry)
					continue;

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
		mutable utils::SpinLock m_lock;
	};
}}
