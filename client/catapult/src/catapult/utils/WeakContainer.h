#pragma once
#include "SpinLock.h"
#include <list>
#include <memory>
#include <mutex>

namespace catapult { namespace utils {

	/// A container of weak_ptr<T> pointing to closable items.
	template<typename T>
	class WeakContainer {
	private:
		using SpinLockGuard = std::lock_guard<utils::SpinLock>;

	public:
		/// Creates an empty container.
		WeakContainer() : WeakContainer([](const auto&) {})
		{}

		/// Creates an empty container with a custom close function (\a close).
		WeakContainer(const std::function<void (T&)>& close) : m_close(close)
		{}

	public:
		/// Gets the number of items in the container and removes all previously deleted items.
		size_t size() const {
			SpinLockGuard guard(m_lock);
			const_cast<WeakContainer*>(this)->pruneInternal();
			return m_entities.size();
		}

		/// Adds \a pEntity to this container and removes all previously deleted items.
		void insert(const std::weak_ptr<T>& pEntity) {
			SpinLockGuard guard(m_lock);
			pruneInternal();
			m_entities.push_back(pEntity);
		}

		/// Closes and removes all items in this container.
		void clear() {
			SpinLockGuard guard(m_lock);
			for (const auto& pEntity : m_entities) {
				auto pSharedEntity = pEntity.lock();
				if (!pSharedEntity)
					continue;

				m_close(*pSharedEntity);
			}

			m_entities.clear();
		}

	private:
		void pruneInternal() {
			m_entities.remove_if([](const auto& pExistingEntity) { return !pExistingEntity.lock(); });
		}

	private:
		std::function<void (T&)> m_close;
		std::list<std::weak_ptr<T>> m_entities;
		mutable utils::SpinLock m_lock;
	};
}}
