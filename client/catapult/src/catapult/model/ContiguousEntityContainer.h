#pragma once
#include "catapult/exceptions.h"
#include "catapult/types.h"

namespace catapult { namespace model {

	/// Polcies for handling iteration errors.
	enum class EntityContainerErrorPolicy {
		/// Immediately throw when an iteration error is encounted.
		Throw,
		/// Do not throw when an iteration error is encountered (but error flag will still be set).
		Suppress
	};

	/// Container wrapper around contiguous memory structures that have a Size field
	/// indicating their size in bytes.
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
		/// The actual iterator.
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

			static constexpr auto Advance(value_type* pEntity, size_t numBytes) {
				return reinterpret_cast<value_type*>(ToBytePointer(pEntity) + numBytes);
			}

		public:
			/// Creates an iterator around \a pStart and \a state with specified current position (\a pCurrent).
			iterator(value_type* pStart, value_type* pCurrent, State& state)
					: m_pStart(pStart)
					, m_pCurrent(pCurrent ? pCurrent : Advance(m_pStart, state.Size))
					, m_state(state) {
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

				m_pCurrent = Advance(m_pCurrent, m_pCurrent->Size);
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
			/// Returns a reference to the current entity.
			reference operator*() const {
				return *(this->operator->());
			}

			/// Returns a pointer to the current entity.
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
				return endBytePointer() == ToBytePointer(pEntity);
			}

			constexpr auto endBytePointer() const noexcept {
				return ToBytePointer(m_pStart) + m_state.Size;
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
		/// Returns a const iterator that represents the first entity.
		auto cbegin() const {
			return iterator<const TEntity>(m_pStart, m_pStart, m_state);
		}

		/// Returns a const iterator that represents one past the last entity.
		auto cend() const {
			return iterator<const TEntity>(m_pStart, nullptr, m_state);
		}

		/// Returns an iterator that represents the first entity.
		auto begin() const {
			return iterator<TEntity>(m_pStart, m_pStart, m_state);
		}

		/// Returns an iterator that represents one past the last entity.
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
