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
#include "BaseSetCommitPolicy.h"
#include "DeltaElements.h"
#include <memory>

namespace catapult { namespace deltaset {

	namespace detail {
		// need to expose hashed or sorted container traits as hint to key storage in BaseSetDelta secondary containers
		template<typename T, typename = void>
		struct StlContainerTraits {
			using key_compare = typename T::key_compare;
		};

		template<typename T>
		struct StlContainerTraits<T, utils::traits::is_type_expression_t<typename T::hasher>> {
			using hasher = typename T::hasher;
			using key_equal = typename T::key_equal;
		};
	}

	/// Possible conditional container modes.
	enum class ConditionalContainerMode {
		/// Delegate to storage.
		Storage,

		/// Delegate to memory.
		Memory
	};

	/// Conditional container that delegates to either a storage or a memory backed container.
	template<typename TKeyTraits, typename TStorageSet, typename TMemorySet>
	class ConditionalContainer : public detail::StlContainerTraits<TMemorySet> {
	public:
		using StorageSetType = TStorageSet;
		using MemorySetType = TMemorySet;

		using value_type = typename MemorySetType::value_type;

	private:
		// use flags to support similar storage and memory sets with same iterator
		using StorageFlag = std::integral_constant<ConditionalContainerMode, ConditionalContainerMode::Storage>;
		using MemoryFlag = std::integral_constant<ConditionalContainerMode, ConditionalContainerMode::Memory>;

	public:
		/// Const iterator.
		class ConditionalIterator {
		public:
			/// Creates an uninitialized iterator.
			ConditionalIterator() = default;

			/// Creates a conditional iterator around \a iter for a storage container.
			ConditionalIterator(typename StorageSetType::const_iterator&& iter, StorageFlag)
					: m_storageIter(std::move(iter))
					, m_mode(ConditionalContainerMode::Storage)
			{}

			/// Creates a conditional iterator around \a iter for a memory container.
			ConditionalIterator(typename MemorySetType::const_iterator&& iter, MemoryFlag)
					: m_memoryIter(std::move(iter))
					, m_mode(ConditionalContainerMode::Memory)
			{}

		public:
			/// Returns \c true if this iterator is equal to \a rhs.
			bool operator==(const ConditionalIterator& rhs) const {
				return ConditionalContainerMode::Storage == m_mode ? m_storageIter == rhs.m_storageIter : m_memoryIter == rhs.m_memoryIter;
			}

			/// Returns \c true if this iterator is not equal to \a rhs.
			bool operator!=(const ConditionalIterator& rhs) const {
				return !(*this == rhs);
			}

		public:
			/// Gets a const pointer to the current element.
			const auto* operator->() const {
				return &operator*();
			}

			/// Gets a const reference to the current element.
			const auto& operator*() const {
				return ConditionalContainerMode::Storage == m_mode ? *m_storageIter : *m_memoryIter;
			}

		private:
			typename StorageSetType::const_iterator m_storageIter;
			typename MemorySetType::const_iterator m_memoryIter;
			ConditionalContainerMode m_mode;
		};

	public:
		using const_iterator = ConditionalIterator;
		using iterator = ConditionalIterator;

	public:
		/// Creates a memory conditional container with \a mode.
		ConditionalContainer() : ConditionalContainer(ConditionalContainerMode::Memory)
		{}

		/// Creates a memory conditional container with \a mode.
		/// \a storageArgs are forwarded to the underlying storage container.
		template<typename... TStorageArgs>
		explicit ConditionalContainer(ConditionalContainerMode mode, TStorageArgs&&... storageArgs) {
			if (ConditionalContainerMode::Storage == mode)
				m_pContainer1 = std::make_unique<StorageSetType>(std::forward<TStorageArgs>(storageArgs)...);
			else
				m_pContainer2 = std::make_unique<MemorySetType>();
		}

	public:
		/// Gets a value indicating whether or not this set is empty.
		bool empty() const {
			return m_pContainer1 ? m_pContainer1->empty() : m_pContainer2->empty();
		}

		/// Gets the size of this set.
		size_t size() const {
			return m_pContainer1 ? m_pContainer1->size() : m_pContainer2->size();
		}

	public:
		/// Gets a const iterator to the element following the last element of the underlying set.
		ConditionalIterator cend() const {
			return m_pContainer1
					? ConditionalIterator(m_pContainer1->cend(), StorageFlag())
					: ConditionalIterator(m_pContainer2->cend(), MemoryFlag());
		}

		/// Searches for \a key in this set.
		ConditionalIterator find(const typename TKeyTraits::KeyType& key) const {
			return m_pContainer1
					? ConditionalIterator(m_pContainer1->find(key), StorageFlag())
					: ConditionalIterator(m_pContainer2->find(key), MemoryFlag());
		}

	public:
		/// Applies all changes in \a deltas to the underlying container.
		void update(const DeltaElements<MemorySetType>& deltas) {
			if (m_pContainer1)
				UpdateSet<TKeyTraits>(*m_pContainer1, deltas);
			else
				UpdateSet<TKeyTraits>(*m_pContainer2, deltas);
		}

		/// Optionally prunes underlying container using \a pruningBoundary.
		template<typename TPruningBoundary>
		void prune(const TPruningBoundary& pruningBoundary) {
			if (m_pContainer1)
				PruneBaseSet(*m_pContainer1, pruningBoundary);
			else
				PruneBaseSet(*m_pContainer2, pruningBoundary);
		}

	private:
		std::unique_ptr<StorageSetType> m_pContainer1;
		std::unique_ptr<MemorySetType> m_pContainer2;

	private:
		template<typename TKeyTraits2, typename TStorageSet2, typename TMemorySet2>
		friend bool IsSetIterable(const ConditionalContainer<TKeyTraits2, TStorageSet2, TMemorySet2>& set);

		template<typename TKeyTraits2, typename TStorageSet2, typename TMemorySet2>
		friend const TMemorySet2& SelectIterableSet(const ConditionalContainer<TKeyTraits2, TStorageSet2, TMemorySet2>& set);
	};

	/// Returns \c true if \a set is iterable.
	/// \note Specialization for ConditionalContainer.
	template<typename TKeyTraits, typename TStorageSet, typename TMemorySet>
	bool IsSetIterable(const ConditionalContainer<TKeyTraits, TStorageSet, TMemorySet>& set) {
		return !!set.m_pContainer2;
	}

	/// Selects the iterable set from \a set.
	/// \throws catapult_invalid_argument if the set is not memory-based.
	/// \note Specialization for ConditionalContainer.
	template<typename TKeyTraits, typename TStorageSet, typename TMemorySet>
	const TMemorySet& SelectIterableSet(const ConditionalContainer<TKeyTraits, TStorageSet, TMemorySet>& set) {
		if (!IsSetIterable(set))
			CATAPULT_THROW_INVALID_ARGUMENT("ConditionalContainer is only iterable when it is memory-based");

		return *set.m_pContainer2;
	}

	/// Applies all changes in \a deltas to \a container.
	/// \note Specialization for ConditionalContainer.
	template<typename TKeyTraits, typename TStorageSet, typename TMemorySet>
	void UpdateSet(ConditionalContainer<TKeyTraits, TStorageSet, TMemorySet>& container, const DeltaElements<TMemorySet>& deltas) {
		container.update(deltas);
	}

	/// Optionally prunes \a elements using \a pruningBoundary, which indicates the upper bound of elements to remove.
	/// \note Specialization for ConditionalContainer.
	template<typename TKeyTraits, typename TStorageSet, typename TMemorySet, typename TPruningBoundary>
	void PruneBaseSet(ConditionalContainer<TKeyTraits, TStorageSet, TMemorySet>& container, const TPruningBoundary& pruningBoundary) {
		container.prune(pruningBoundary);
	}
}}
