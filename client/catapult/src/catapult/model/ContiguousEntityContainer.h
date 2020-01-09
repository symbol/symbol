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
#include "catapult/utils/IntegerMath.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"

namespace catapult { namespace model {

	/// Policies for handling iteration errors.
	enum class EntityContainerErrorPolicy {
		/// Immediately throw when an iteration error is encounted.
		Throw,

		/// Do not throw when an iteration error is encountered (but error flag will still be set).
		Suppress
	};

	/// Container wrapper around contiguous memory structures that have a Size field indicating size in bytes.
	/// \note Container assumes structures are aligned on 8-byte boundaries.
	template<typename TEntity>
	class BasicContiguousEntityContainer final {
	public:
		using value_type = TEntity;

	public:
		/// Creates a container around \a pEntity structures spanning over \a entitiesSize bytes with the specified
		/// error policy (\a errorPolicy).
		constexpr BasicContiguousEntityContainer(TEntity* pEntity, size_t entitiesSize, EntityContainerErrorPolicy errorPolicy)
				: m_pStart(pEntity)
				, m_state(entitiesSize, EntityContainerErrorPolicy::Throw == errorPolicy)
		{}

	private:
		struct State {
		public:
			const size_t Size;
			const bool ThrowOnError;
			bool HasError;

		public:
			constexpr State(size_t size, bool throwOnError)
					: Size(size)
					, ThrowOnError(throwOnError)
					, HasError(false)
			{}
		};

	public:
		/// Actual iterator.
		template<typename TIteratorEntity>
		class iterator {
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = TIteratorEntity;
			using pointer = TIteratorEntity*;
			using reference = TIteratorEntity&;
			using iterator_category = std::forward_iterator_tag;

		private:
			template<typename T>
			static constexpr auto ToBytePointer(T* pEntity) {
				return reinterpret_cast<uint8_t*>(pEntity);
			}

			template<typename T>
			static constexpr auto ToBytePointer(const T* pEntity) {
				return reinterpret_cast<const uint8_t*>(pEntity);
			}

		public:
			/// Creates an iterator around \a pStart and \a state with specified current position (\a pCurrent).
			iterator(value_type* pStart, value_type* pCurrent, State& state)
					: m_pStart(pStart)
					, m_pCurrent(pCurrent)
					, m_state(state) {
				// only advance when m_pStart is a valid poiner (m_pStart may be nullptr when empty)
				if (!m_pCurrent && m_pStart)
					m_pCurrent = advance(m_pStart, m_state.Size); // advance to end

				checkError();
			}

		public:
			/// Returns \c true if this iterator and \a rhs are equal.
			bool operator==(const iterator& rhs) const {
				return m_pStart == rhs.m_pStart && m_state.Size == rhs.m_state.Size && m_pCurrent == rhs.m_pCurrent;
			}

			/// Returns \c true if this iterator and \a rhs are not equal.
			bool operator!=(const iterator& rhs) const {
				return !(*this == rhs);
			}

		public:
			/// Advances the iterator to the next position.
			iterator& operator++() {
				if (isEnd(m_pCurrent))
					CATAPULT_THROW_OUT_OF_RANGE("cannot advance iterator beyond end");

				m_pCurrent = advance(m_pCurrent, m_pCurrent->Size);
				if (isEnd(m_pCurrent))
					return *this;

				checkError();
				return *this;
			}

			/// Advances the iterator to the next position.
			iterator operator++(int) {
				auto copy = *this;
				++*this;
				return copy;
			}

		public:
			/// Gets a reference to the current entity.
			reference operator*() const {
				return *(this->operator->());
			}

			/// Gets a pointer to the current entity.
			pointer operator->() const {
				if (isEnd(m_pCurrent))
					CATAPULT_THROW_OUT_OF_RANGE("cannot dereference at end");

				return m_pCurrent;
			}

		private:
			constexpr bool isEntityInBuffer(value_type* pEntity) const noexcept {
				return
						ToBytePointer(pEntity) <= endBytePointer() &&
						ToBytePointer(pEntity) + sizeof(pEntity->Size) <= endBytePointer() && // ensure Size is readable
						ToBytePointer(pEntity) + pEntity->Size <= endBytePointer();
			}

			constexpr bool isEnd(value_type* pEntity) const noexcept {
				return !pEntity || endBytePointer() == ToBytePointer(pEntity);
			}

			constexpr auto endBytePointer() const noexcept {
				return ToBytePointer(m_pStart) + m_state.Size;
			}

			auto advance(value_type* pEntity, size_t numBytes) {
				auto* pNextEntityBytes = ToBytePointer(pEntity) + numBytes;

				// if entity is not last one, it must be padded to an 8-byte boundary (last entity is not padded)
				if (endBytePointer() != pNextEntityBytes)
					pNextEntityBytes += utils::GetPaddingSize(numBytes, 8);

				return reinterpret_cast<value_type*>(pNextEntityBytes);
			}

			void checkError() {
				if (isEnd(m_pCurrent) || (isEntityInBuffer(m_pCurrent) && m_pCurrent->Size >= sizeof(TEntity)))
					return;

				m_pCurrent = reinterpret_cast<value_type*>(endBytePointer());
				m_state.HasError = true;
				if (m_state.ThrowOnError)
					CATAPULT_THROW_RUNTIME_ERROR("error encountered while iterating");
			}

		private:
			value_type* m_pStart;
			value_type* m_pCurrent;
			State& m_state;
		};

	public:
		/// Gets a const iterator that represents the first entity.
		auto cbegin() const {
			return iterator<const TEntity>(m_pStart, m_pStart, m_state);
		}

		/// Gets a const iterator that represents one past the last entity.
		auto cend() const {
			return iterator<const TEntity>(m_pStart, nullptr, m_state);
		}

		/// Gets an iterator that represents the first entity.
		auto begin() const {
			return iterator<TEntity>(m_pStart, m_pStart, m_state);
		}

		/// Gets an iterator that represents one past the last entity.
		auto end() const {
			return iterator<TEntity>(m_pStart, nullptr, m_state);
		}

	public:
		/// Gets a value indicating whether or not there was an iteration error.
		bool hasError() const {
			return m_state.HasError;
		}

	private:
		TEntity* m_pStart;
		mutable State m_state;
	};

	/// Creates a container over memory pointed to by \a pEntity spanning across \a size bytes with the
	/// desired error policy (\a errorPolicy).
	template<typename TEntity>
	constexpr BasicContiguousEntityContainer<TEntity> MakeContiguousEntityContainer(
			TEntity* pEntity,
			size_t size,
			EntityContainerErrorPolicy errorPolicy = EntityContainerErrorPolicy::Throw) {
		return BasicContiguousEntityContainer<TEntity>(pEntity, size, errorPolicy);
	}
}}
