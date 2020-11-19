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
#include "catapult/constants.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Compact array-based stack that allocates memory dynamically only when it is not empty.
	template<typename T, size_t N>
	class CompactArrayStack {
	public:
		// region const_iterator

		/// Compact array stack const iterator.
		/// \note Iterator will always return N values to mimic std::array behavior.
		/// \note Custom iterator is needed in order to support iteration when std::array member is \c nullptr (memory optimization).
		class const_iterator {
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = const T;
			using pointer = const T*;
			using reference = const T&;
			using iterator_category = std::forward_iterator_tag;

		public:
			/// Creates an iterator around \a pArray with \a index current position.
			const_iterator(const std::array<T, N>* pArray, size_t index)
					: m_pArray(pArray)
					, m_index(index)
			{}

		public:
			/// Returns \c true if this iterator and \a rhs are equal.
			bool operator==(const const_iterator& rhs) const {
				return m_pArray == rhs.m_pArray && m_index == rhs.m_index;
			}

			/// Returns \c true if this iterator and \a rhs are not equal.
			bool operator!=(const const_iterator& rhs) const {
				return !(*this == rhs);
			}

		public:
			/// Advances the iterator to the next position.
			const_iterator& operator++() {
				if (isEnd())
					CATAPULT_THROW_OUT_OF_RANGE("cannot advance iterator beyond end");

				++m_index;
				return *this;
			}

			/// Advances the iterator to the next position.
			const_iterator operator++(int) {
				auto copy = *this;
				++*this;
				return copy;
			}

		public:
			/// Gets a reference to the current value.
			reference operator*() const {
				return *(this->operator->());
			}

			/// Gets a pointer to the current value.
			pointer operator->() const {
				if (isEnd())
					CATAPULT_THROW_OUT_OF_RANGE("cannot dereference at end");

				return m_pArray ? &(*m_pArray)[m_index] : &m_defaultValue;
			}

		private:
			bool isEnd() const {
				return N == m_index;
			}

		private:
			const std::array<T, N>* m_pArray;
			size_t m_index;
			T m_defaultValue;
		};

		// endregion

	public:
		/// Creates an empty stack.
		CompactArrayStack() : m_size(0)
		{}

		/// Copy constructor that makes a deep copy of \a stack.
		CompactArrayStack(const CompactArrayStack& stack) {
			*this = stack;
		}

		/// Move constructor that move constructs a stack from \a stack.
		CompactArrayStack(CompactArrayStack&& stack)
				: m_pArray(std::move(stack.m_pArray))
				, m_size(stack.m_size) {
			stack.m_size = 0;
		}

	public:
		/// Assignment operator that makes a deep copy of \a stack.
		CompactArrayStack& operator=(const CompactArrayStack& stack) {
			if (stack.m_pArray)
				m_pArray = std::make_unique<std::array<T, N>>(*stack.m_pArray);

			m_size = stack.m_size;
			return *this;
		}

		/// Move assignment operator that assigns \a stack.
		CompactArrayStack& operator=(CompactArrayStack&& stack) {
			m_pArray = std::move(stack.m_pArray);
			m_size = stack.m_size;
			stack.m_size = 0;
			return *this;
		}

	public:
		/// Gets the number of non-default values contained in the stack.
		size_t size() const {
			return m_size;
		}

	public:
		/// Gets a const iterator to the first element of the underlying container.
		const_iterator begin() const {
			return const_iterator(m_pArray.get(), 0);
		}

		/// Gets a const iterator to the element following the last element of the underlying container.
		const_iterator end() const {
			return const_iterator(m_pArray.get(), N);
		}

	public:
		/// Gets a const reference to the element on the top of the stack.
		const T& peek() const {
			if (0 == m_size)
				CATAPULT_THROW_OUT_OF_RANGE("cannot peek when empty");

			return m_pArray->front();
		}

		/// Gets a reference to the element on the top of the stack.
		T& peek() {
			if (0 == m_size)
				CATAPULT_THROW_OUT_OF_RANGE("cannot peek when empty");

			return m_pArray->front();
		}

	public:
		/// Pushes \a value onto the stack.
		void push(const T& value) {
			shiftRight();
			if (m_size < N)
				++m_size;

			if (!m_pArray)
				m_pArray = std::make_unique<std::array<T, N>>();

			m_pArray->front() = value;
		}

		/// Pops the top value from the stack.
		void pop() {
			shiftLeft();
			m_pArray->back() = T();
			--m_size;

			if (0 == m_size)
				m_pArray.reset();
		}

	private:
		void shiftLeft() {
			if (!m_pArray)
				CATAPULT_THROW_OUT_OF_RANGE("cannot pop when empty");

			auto& array = *m_pArray;
			for (auto i = 0u; i < array.size() - 1; ++i)
				array[i] = array[i + 1];
		}

		void shiftRight() {
			if (!m_pArray)
				return;

			auto& array = *m_pArray;
			for (auto i = array.size() - 1; i > 0; --i)
				array[i] = array[i - 1];
		}

	private:
		std::unique_ptr<std::array<T, N>> m_pArray;
		size_t m_size;
	};
}}
