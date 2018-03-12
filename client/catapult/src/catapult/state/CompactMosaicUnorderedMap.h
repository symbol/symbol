#pragma once
#include "catapult/utils/Hashers.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"
#include <unordered_map>

namespace catapult { namespace state {

	/// A mosaic unordered map that is optimized for storage of a small number of elements.
	/// \note This map assumes that MosaicId(0) is not a valid mosaic.
	//        This is acceptable for mosaics stored in AccountBalances but not for a general purpose map.
	class CompactMosaicUnorderedMap : utils::MoveOnly {
	private:
		static constexpr auto Array_Size = 5;

		// in order for this map to behave like std::unordered_map, the element type needs to be a pair, not model::Mosaic
		using Mosaic = std::pair<const MosaicId, Amount>;
		using MutableMosaic = std::pair<MosaicId, Amount>;
		using MosaicArray = std::array<MutableMosaic, Array_Size>;
		using MosaicMap = std::unordered_map<MosaicId, Amount, utils::BaseValueHasher<MosaicId>>;

	private:
		struct SecondLevelStorage {
			MosaicArray ArrayStorage;
			uint8_t ArraySize;
			std::unique_ptr<MosaicMap> pMapStorage;
		};

		struct FirstLevelStorage {
		public:
			MutableMosaic Value;
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
		/// The base of mosaic iterators.
		class basic_iterator {
		public:
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

		public:
			/// The iterator stage.
			enum class Stage { Start, Value, Array, Map, End };

		public:
			/// Creates an iterator around \a storage with initial \a stage.
			explicit basic_iterator(FirstLevelStorage& storage, Stage stage);

			/// Creates an iterator around \a storage pointing to the mosaic at \a location.
			explicit basic_iterator(FirstLevelStorage& storage, const MosaicLocation& location);

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

		/// A basic typed iterator that adds support for dereferencing.
		template<typename T>
		class basic_iterator_t : public basic_iterator {
		public:
			using value_type = T;
			using pointer = value_type*;
			using reference = value_type&;

		public:
			using basic_iterator::basic_iterator;

		public:
			/// Returns a reference to the current value.
			reference operator*() const {
				return current();
			}

			/// Returns a pointer to the current value.
			pointer operator->() const {
				return &current();
			}
		};

	public:
		/// The mosaic const iterator.
		using const_iterator = basic_iterator_t<const Mosaic>;

		/// The mosaic non-const iterator.
		using iterator = basic_iterator_t<Mosaic>;

	public:
		/// Returns a const iterator to the first element of the underlying container.
		const_iterator begin() const;

		/// Returns a const iterator to the element following the last element of the underlying container.
		const_iterator end() const;

		/// Returns an iterator to the first element of the underlying container.
		iterator begin();

		/// Returns an iterator to the element following the last element of the underlying container.
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

	private:
		bool find(MosaicId id, MosaicLocation& location) const;

		void compact();

	private:
		FirstLevelStorage m_storage;
	};
}}
