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
#include "KeySerializers.h"
#include "RdbColumnContainer.h"
#include "RocksDatabase.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"

namespace catapult { namespace cache {

	/// Typed container adapter that wraps column.
	template<typename TDescriptor, typename TContainer = RdbColumnContainer>
	class RdbTypedColumnContainer : public TContainer {
	public:
		using KeyType = typename TDescriptor::KeyType;
		using ValueType = typename TDescriptor::ValueType;
		using StorageType = typename TDescriptor::StorageType;

	public:
		/// Typed container iterator that adds descriptor-based deserialization.
		class const_iterator {
		private:
			using KeyType = typename TDescriptor::KeyType;
			using ValueType = typename TDescriptor::ValueType;
			using StorageType = typename TDescriptor::StorageType;

		public:
			/// Returns \c true if this iterator and \a rhs are equal.
			bool operator==(const const_iterator& rhs) const {
				return m_iterator == rhs.m_iterator;
			}

			/// Returns \c true if this iterator and \a rhs are not equal.
			bool operator!=(const const_iterator& rhs) const {
				return !(*this == rhs);
			}

		public:
			/// Gets a reference to current element.
			const StorageType& operator*() const {
				if (RdbDataIterator::End() == m_iterator)
					CATAPULT_THROW_INVALID_ARGUMENT("dereference on empty iterator");

				if (!m_pStorage) {
					auto value = TDescriptor::Serializer::DeserializeValue(m_iterator.buffer());
					m_pStorage = std::make_unique<StorageType>(TDescriptor::ToStorage(value));
				}

				return *m_pStorage;
			}

			/// Gets a pointer to current element.
			const StorageType* operator->() const {
				return &operator*();
			}

		public:
			/// Gets a const reference to underlying db iterator.
			const RdbDataIterator& dbIterator() const {
				return m_iterator;
			}

			/// Gets a reference to underlying db iterator.
			RdbDataIterator& dbIterator() {
				return m_iterator;
			}

		private:
			RdbDataIterator m_iterator;
			mutable std::shared_ptr<StorageType> m_pStorage;
		};

	public:
		/// Creates a container around \a database and \a columnId.
		template<typename TDatabase = RocksDatabase>
		RdbTypedColumnContainer(TDatabase& database, size_t columnId) : TContainer(database, columnId)
		{}

	public:
		/// Returns \c true if container is empty.
		bool empty() const {
			return 0 == TContainer::size();
		}

	public:

#if !defined(NDEBUG) && defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4702) /* "unreachable code" */
#endif

		/// Inserts \a element into container.
		void insert(const StorageType& element) {
			TContainer::insert(
					SerializeKey(TDescriptor::ToKey(element)),
					TDescriptor::Serializer::SerializeValue(TDescriptor::ToValue(element)));
		}

#if !defined(NDEBUG) && defined(_MSC_VER)
#pragma warning(pop)
#endif

		/// Finds element with \a key. Returns cend() if \a key has not been found.
		const_iterator find(const KeyType& key) const {
			const_iterator iter;
			TContainer::find(SerializeKey(key), iter.dbIterator());
			return iter;
		}

		/// Prunes elements with keys smaller than \a key. Returns number of pruned elements.
		size_t prune(const KeyType& key) {
			return TContainer::prune(TDescriptor::Serializer::KeyToBoundary(key));
		}

		/// Removes element with \a key.
		void remove(const KeyType& key) {
			TContainer::remove(SerializeKey(key));
		}

		/// Gets an iterator that represents non-existing element.
		const_iterator cend() const {
			return const_iterator();
		}
	};
}}
