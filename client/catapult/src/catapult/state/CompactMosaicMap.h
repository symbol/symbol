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
#include "catapult/utils/Hashers.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"
#include <map>

namespace catapult { namespace state {

	/// Mosaic (ordered) map that is optimized for storage of a small number of elements.
	/// \note This map assumes that MosaicId(0) is not a valid mosaic.
	///       This is acceptable for mosaics stored in AccountBalances but not for a general purpose map.
	class CompactMosaicMap : utils::MoveOnly {
	private:
		static constexpr auto Array_Size = 5;

		// in order for this map to behave like std::unordered_map, the element type needs to be a pair, not model::Mosaic
		using Mosaic = std::pair<const MosaicId, Amount>;
		using MutableMosaic = std::pair<MosaicId, Amount>;

		// gcc strict aliasing workaround because pair<X, Y> and pair<const X, Y> are two different types
		struct MosaicUnion {
		public:
			MosaicUnion() : Mosaic()
			{}

			MosaicUnion(MosaicUnion&& rhs) : Mosaic(std::move(rhs.Mosaic))
			{}

		public:
			MosaicUnion& operator=(MosaicUnion&& rhs) {
				Mosaic = std::move(rhs.Mosaic);
				return *this;
			}

		public:
			union {
				MutableMosaic Mosaic;
				CompactMosaicMap::Mosaic ConstMosaic;
			};
		};

		using MosaicArray = std::array<MosaicUnion, Array_Size>;
		using MosaicMap = std::map<MosaicId, Amount>;

	private:
		struct SecondLevelStorage {
			MosaicArray ArrayStorage;
			uint8_t ArraySize;
			std::unique_ptr<MosaicMap> pMapStorage;
		};

		struct FirstLevelStorage {
		public:
			MosaicUnion Value;
			std::unique_ptr<SecondLevelStorage> pNextStorage;

		public:
			bool hasValue() const;

			bool hasArray() const;

			bool hasMap() const;

		public:
			uint8_t& arraySize() const;

			MosaicArray& array() const;

			MosaicMap& map() const;
		};

	private:
		enum class MosaicSource { Value, Array, Map };

		struct MosaicLocation {
		public:
			MosaicLocation()
					: Source(static_cast<MosaicSource>(-1))
					, ArrayIndex(0)
					, MapIterator()
			{}

		public:
			MosaicSource Source;
			size_t ArrayIndex;
			MosaicMap::iterator MapIterator;
		};

	private:
		/// Base of mosaic iterators.
		class basic_iterator {
		public:
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

		public:
			/// Iterator stage.
			enum class Stage { Start, Value, Array, Map, End };

		public:
			/// Creates an iterator around \a storage with initial \a stage.
			basic_iterator(FirstLevelStorage& storage, Stage stage);

			/// Creates an iterator around \a storage pointing to the mosaic at \a location.
			basic_iterator(FirstLevelStorage& storage, const MosaicLocation& location);

		public:
			/// Returns \c true if this iterator and \a rhs are equal.
			bool operator==(const basic_iterator& rhs) const;

			/// Returns \c true if this iterator and \a rhs are not equal.
			bool operator!=(const basic_iterator& rhs) const;

		public:
			/// Advances the iterator to the next position.
			basic_iterator& operator++();

			/// Advances the iterator to the next position.
			basic_iterator operator++(int);

		protected:
			/// Gets a reference to the current value.
			Mosaic& current() const;

		private:
			void advance();

			void setValueMosaic();

			void setArrayMosaic();

			void setMapMosaic();

			void setEnd();

			bool isEnd() const;

		private:
			FirstLevelStorage& m_storage;
			Stage m_stage;
			Mosaic* m_pCurrent;

			// indexing into sub containers
			size_t m_arrayIndex;
			MosaicMap::iterator m_mapIterator;
		};

		/// Basic typed iterator that adds support for dereferencing.
		template<typename T>
		class basic_iterator_t : public basic_iterator {
		public:
			using value_type = T;
			using pointer = value_type*;
			using reference = value_type&;

		public:
			using basic_iterator::basic_iterator;

		public:
			/// Gets a reference to the current value.
			reference operator*() const {
				return current();
			}

			/// Gets a pointer to the current value.
			pointer operator->() const {
				return &current();
			}
		};

	public:
		/// Mosaic const iterator.
		using const_iterator = basic_iterator_t<const Mosaic>;

		/// Mosaic non-const iterator.
		using iterator = basic_iterator_t<Mosaic>;

	public:
		/// Gets a const iterator to the first element of the underlying container.
		const_iterator begin() const;

		/// Gets a const iterator to the element following the last element of the underlying container.
		const_iterator end() const;

		/// Gets an iterator to the first element of the underlying container.
		iterator begin();

		/// Gets an iterator to the element following the last element of the underlying container.
		iterator end();

	public:
		/// Returns \c true if the map is empty.
		bool empty() const;

		/// Gets the number of mosaics in the map.
		size_t size() const;

		/// Finds the mosaic with \a id.
		const_iterator find(MosaicId id) const;

		/// Finds the mosaic with \a id.
		iterator find(MosaicId id);

		/// Inserts a mosaic \a pair.
		void insert(const Mosaic& pair);

		/// Erases the mosaic with \a id.
		void erase(MosaicId id);

		/// Optimizes access of the mosaic with \a id.
		void optimize(MosaicId id);

	private:
		bool find(MosaicId id, MosaicLocation& location) const;

		void insertIntoArray(size_t index, const Mosaic& pair);

		void insertIntoMap(const Mosaic& pair);

		void eraseFromArray(size_t index);

		void compact();

	private:
		FirstLevelStorage m_storage;
		MosaicId m_optimizedMosaicId;
	};
}}
