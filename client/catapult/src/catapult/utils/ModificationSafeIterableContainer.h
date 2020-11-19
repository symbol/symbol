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
#include <stddef.h>

namespace catapult { namespace utils {

	/// Wraps a container type and provides modification-safe iteration support.
	template<typename TContainer>
	class ModificationSafeIterableContainer {
	public:
		using value_type = typename TContainer::value_type;
		using iterator = typename TContainer::iterator;

	public:
		/// Gets the number of elements in the container.
		size_t size() const {
			return m_container.size();
		}

		/// Gets a value indicating whether or not the container is empty.
		bool empty() const {
			return m_container.empty();
		}

	public:
		/// Gets a const iterator that represents the first entity.
		auto cbegin() const {
			return m_container.cbegin();
		}

		/// Gets a const iterator that represents one past the last entity.
		auto cend() const {
			return m_container.cend();
		}

		/// Gets an iterator that represents the first entity.
		auto begin() {
			return m_container.begin();
		}

		/// Gets an iterator that represents one past the last entity.
		auto end() {
			return m_container.end();
		}

	public:
		/// Gets a pointer to the next value or \c nullptr if this container is empty.
		value_type* next() {
			if (empty())
				return nullptr;

			if (m_container.end() == m_iter)
				m_iter = m_container.begin();

			auto pNext = &*m_iter;
			++m_iter;
			return pNext;
		}

		/// Gets a pointer to the next value for which \a predicate returns \c true or \c nullptr if this container
		/// contains no matching values.
		template<typename TPredicate>
		value_type* nextIf(TPredicate predicate) {
			auto pValue = next();
			if (!pValue)
				return nullptr;

			auto pFirstValue = pValue;
			while (!predicate(*pValue)) {
				pValue = next();

				// all values failed the predicate
				if (pFirstValue == pValue)
					return nullptr;
			}

			return pValue;
		}

	public:
		/// Adds a new element at the end of the container.
		void push_back(const value_type& value) {
			auto isEmpty = m_container.empty();
			m_container.push_back(value);

			if (isEmpty)
				m_iter = m_container.begin();
		}

		/// Removes a single element at \a position from the container.
		void erase(iterator position) {
			if (m_iter == position) {
				if (m_container.end() == m_iter)
					m_iter = m_container.begin();
				else
					++m_iter;
			}

			m_container.erase(position);
		}

		/// Removes all elements from the container.
		void clear() {
			m_container.clear();
		}

	private:
		TContainer m_container;
		iterator m_iter;
	};
}}
