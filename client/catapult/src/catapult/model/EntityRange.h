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
#include "catapult/utils/IntegerMath.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/exceptions.h"
#include <memory>
#include <vector>

namespace catapult {
	namespace model {
		template<typename TEntity>
		class EntityRange;

		template<typename TEntity>
		class EntityRangeFactoryMixin;
	}
}

namespace catapult { namespace model {

	/// Backing storage for a range of entities.
	template<typename TEntity>
	class EntityRangeStorage : public utils::MoveOnly {
	private:
		// region SubRange

		class SubRange {
		public:
			SubRange() : SubRange(0)
			{}

			explicit SubRange(size_t numBytes) : m_numBytes(numBytes)
			{}

			SubRange(SubRange&& rhs)
					: m_numBytes(rhs.m_numBytes)
					, m_entities(std::move(rhs.m_entities)) {
				rhs.reset();
			}

		public:
			SubRange& operator=(SubRange&& rhs) {
				m_numBytes = rhs.m_numBytes;
				m_entities = std::move(rhs.m_entities);
				rhs.reset();
				return *this;
			}

		public:
			constexpr bool empty() const {
				return 0 == m_numBytes;
			}

			size_t size() const {
				return m_entities.size();
			}

			constexpr size_t totalSize() const {
				return m_numBytes;
			}

			constexpr auto& entities() const {
				return m_entities;
			}

		public:
			auto& entities() {
				return m_entities;
			}

			void reset() {
				m_numBytes = 0;
				m_entities.clear();
			}

		private:
			size_t m_numBytes;
			std::vector<TEntity*> m_entities;
		};

		// endregion

		// region SingleBufferRange

		class SingleBufferRange : public SubRange {
		public:
			SingleBufferRange() : SubRange()
			{}

			SingleBufferRange(size_t dataSize, const std::vector<size_t>& offsets, uint8_t alignment)
					: SingleBufferRange(nullptr, dataSize, offsets, alignment)
			{}

			SingleBufferRange(const uint8_t* pData, size_t dataSize, const std::vector<size_t>& offsets, uint8_t alignment)
					: SubRange(CalculateTotalSize(dataSize, offsets, alignment))
					, m_buffer(SubRange::totalSize()) {
				size_t totalPadding = 0;
				for (auto i = 0u; i < offsets.size(); ++i) {
					auto offset = offsets[i];
					auto* pDest = &m_buffer[offset + totalPadding - offsets[0]];
					SubRange::entities().push_back(reinterpret_cast<TEntity*>(pDest));

					auto size = (i == offsets.size() - 1 ? dataSize : offsets[i + 1]) - offset;
					if (pData)
						std::memcpy(pDest, &pData[offset], size);

					totalPadding += utils::GetPaddingSize(size, alignment);
				}
			}

		public:
			uint8_t* data() {
				return m_buffer.data();
			}

		private:
			const uint8_t* data() const {
				return m_buffer.data();
			}

		public:
			std::vector<std::shared_ptr<TEntity>> detachEntities() {
				std::vector<std::shared_ptr<TEntity>> entities(SubRange::size());
				auto offsets = generateOffsets();
				auto pBufferShared = std::make_shared<decltype(m_buffer)>(std::move(m_buffer));

				size_t i = 0;
				for (auto offset : offsets) {
					auto pEntity = reinterpret_cast<TEntity*>(&(*pBufferShared)[offset]);
					entities[i++] = std::shared_ptr<TEntity>(pEntity, [pBufferShared](const auto*) {});
				}

				return entities;
			}

		public:
			SingleBufferRange copy() const {
				return SingleBufferRange(data(), SubRange::totalSize(), generateOffsets(), 1);
			}

		private:
			std::vector<size_t> generateOffsets() const {
				size_t i = 0;
				std::vector<size_t> offsets(SubRange::size());
				for (const auto* pEntity : SubRange::entities())
					offsets[i++] = static_cast<size_t>(reinterpret_cast<const uint8_t*>(pEntity) - data());

				return offsets;
			}

		private:
			static size_t CalculateTotalSize(size_t dataSize, const std::vector<size_t>& offsets, uint8_t alignment) {
				// this works because last entity is *not* padded
				if (1 != alignment) {
					for (auto i = 1u; i < offsets.size(); ++i) {
						auto size = offsets[i] - offsets[i - 1];
						dataSize += utils::GetPaddingSize(size, alignment);
					}
				}

				return dataSize - (offsets.empty() ? 0 : offsets[0]);
			}

		private:
			std::vector<uint8_t> m_buffer;
		};

		// endregion

		// region SingleEntityRange

		class SingleEntityRange : public SubRange {
		public:
			SingleEntityRange() : SubRange()
			{}

			explicit SingleEntityRange(std::unique_ptr<TEntity>&& pEntity)
					: SubRange(pEntity->Size)
					, m_pSingleEntity(std::move(pEntity)) {
				SubRange::entities().push_back(m_pSingleEntity.get());
			}

		public:
			std::vector<std::shared_ptr<TEntity>> detachEntities() {
				std::vector<std::shared_ptr<TEntity>> entities(1);
				entities[0] = std::move(m_pSingleEntity);
				return entities;
			}

			SingleBufferRange copy() const {
				return SingleBufferRange(reinterpret_cast<const uint8_t*>(m_pSingleEntity.get()), SubRange::totalSize(), { 0 }, 1);
			}

		private:
			std::shared_ptr<TEntity> m_pSingleEntity;
		};

		// endregion

		// region MultiBufferRange

		class MultiBufferRange : public SubRange {
		public:
			MultiBufferRange() : SubRange()
			{}

			explicit MultiBufferRange(std::vector<EntityRangeStorage>&& ranges)
					: SubRange(CalculateTotalSize(ranges))
					, m_ranges(std::move(ranges)) {
				for (auto& range : m_ranges) {
					for (auto* pEntity : range.subRange().entities())
						SubRange::entities().push_back(pEntity);
				}
			}

		public:
			std::vector<std::shared_ptr<TEntity>> detachEntities() {
				std::vector<std::shared_ptr<TEntity>> allEntities;
				allEntities.reserve(SubRange::size());

				for (auto& range : m_ranges) {
					auto rangeEntities = range.detachSubRangeEntities();
					allEntities.insert(
							allEntities.end(),
							std::make_move_iterator(rangeEntities.begin()),
							std::make_move_iterator(rangeEntities.end()));
				}

				return allEntities;
			}

		public:
			MultiBufferRange copy() const {
				std::vector<EntityRangeStorage> copyRanges;
				for (const auto& range : m_ranges)
					copyRanges.push_back(range.copySubRange());

				return MultiBufferRange(std::move(copyRanges));
			}

		private:
			static size_t CalculateTotalSize(const std::vector<EntityRangeStorage>& ranges) {
				size_t totalSize = 0;
				for (const auto& range : ranges)
					totalSize += range.subRange().totalSize();

				return totalSize;
			}

		private:
			std::vector<EntityRangeStorage> m_ranges;
		};

		// endregion

	public:
		// region constructors

		/// Creates empty storage.
		EntityRangeStorage()
		{}

		/// Creates storage around \a subRange.
		explicit EntityRangeStorage(SingleBufferRange&& subRange) : m_singleBufferRange(std::move(subRange))
		{}

		/// Creates storage around \a subRange.
		explicit EntityRangeStorage(SingleEntityRange&& subRange) : m_singleEntityRange(std::move(subRange))
		{}

		/// Creates storage around \a subRange.
		explicit EntityRangeStorage(MultiBufferRange&& subRange) : m_multiBufferRange(std::move(subRange))
		{}

		// endregion

	public:
		// region helpers

		/// Throws if data is not contiguous.
		void requireContiguousData() const {
			if (!m_multiBufferRange.empty())
				CATAPULT_THROW_RUNTIME_ERROR("data is not accessible when range is composed of non-contiguous data");
		}

		/// Copies the active sub range.
		auto copySubRange() const {
			return activeSubRangeAction([](const auto& subRange) { return EntityRangeStorage(subRange.copy()); });
		}

		/// Gets the active sub range.
		const SubRange& subRange() const {
			return const_cast<EntityRangeStorage&>(*this).subRange();
		}

		/// Gets the active sub range.
		SubRange& subRange() {
			SubRange* pSubRange;
			activeSubRangeAction([&pSubRange](auto& subRange) { pSubRange = &subRange; });
			return *pSubRange;
		}

		/// Detaches all entities from the active sub range.
		auto detachSubRangeEntities() {
			return activeSubRangeAction([](auto& subRange) {
				auto entities = subRange.detachEntities();
				subRange.reset();
				return entities;
			});
		}

		// endregion

	private:
		// region activeSubRangeAction

		template<typename TFunc>
		auto activeSubRangeAction(TFunc func) const {
			return const_cast<EntityRangeStorage&>(*this).activeSubRangeAction(func);
		}

		template<typename TFunc>
		auto activeSubRangeAction(TFunc func) {
			if (!m_singleEntityRange.empty())
				return func(m_singleEntityRange);

			if (!m_multiBufferRange.empty())
				return func(m_multiBufferRange);

			return func(m_singleBufferRange);
		}

		// endregion

	private:
		friend class EntityRangeFactoryMixin<TEntity>;

	private:
		SingleBufferRange m_singleBufferRange;
		SingleEntityRange m_singleEntityRange;
		MultiBufferRange m_multiBufferRange;
	};

	// region EntityRangeFactoryMixin

	/// Mixin that adds EntityRange factory functions.
	template<typename TEntity>
	class EntityRangeFactoryMixin {
	private:
		using Range = EntityRange<TEntity>;
		using RangeStorage = EntityRangeStorage<TEntity>;

		using SingleBufferRange = typename RangeStorage::SingleBufferRange;
		using SingleEntityRange = typename RangeStorage::SingleEntityRange;
		using MultiBufferRange = typename RangeStorage::MultiBufferRange;

	public:
		/// Creates an uninitialized entity range of contiguous memory around \a numElements fixed size elements.
		/// \a ppRangeData is set to point to the range memory.
		static Range PrepareFixed(size_t numElements, uint8_t** ppRangeData = nullptr) {
			std::vector<size_t> offsets(numElements);
			for (auto i = 0u; i < numElements; ++i)
				offsets[i] = i * sizeof(TEntity);

			auto range = Range(RangeStorage(SingleBufferRange(numElements * sizeof(TEntity), offsets, 1)));
			if (ppRangeData)
				*ppRangeData = reinterpret_cast<uint8_t*>(range.data());

			return range;
		}

		/// Creates an entity range around \a numElements fixed size elements pointed to by \a pData.
		static Range CopyFixed(const uint8_t* pData, size_t numElements) {
			uint8_t* pRangeData;
			auto range = PrepareFixed(numElements, &pRangeData);
			utils::memcpy_cond(pRangeData, pData, range.totalSize());
			return range;
		}

		/// Creates an entity range around the data pointed to by \a pData with size \a dataSize and \a offsets
		/// container that contains values indicating the starting position of all entities in the data.
		/// Entities will be aligned to specified \a alignment relative to first offset.
		static Range CopyVariable(const uint8_t* pData, size_t dataSize, const std::vector<size_t>& offsets, uint8_t alignment = 1) {
			return Range(RangeStorage(SingleBufferRange(pData, dataSize, offsets, alignment)));
		}

		/// Creates an entity range around a single entity (\a pEntity).
		static Range FromEntity(std::unique_ptr<TEntity>&& pEntity) {
			return Range(RangeStorage(SingleEntityRange(std::move(pEntity))));
		}

		/// Merges all \a ranges into a single range.
		static Range MergeRanges(std::vector<Range>&& ranges) {
			std::vector<RangeStorage> storages;
			storages.reserve(ranges.size());
			for (auto& range : ranges)
				storages.push_back(std::move(range.m_storage));

			return Range(RangeStorage(MultiBufferRange(std::move(storages))));
		}
	};

	// endregion

	// region EntityRangeIteratorFactory

	template<typename TEntity>
	class EntityRangeIteratorFactory {
	private:
		using value_type = TEntity;

	public:
		/// Entity range iterator.
		template<typename TIterator, typename TIteratorEntity>
		class iterator {
		public:
			using difference_type = typename TIterator::difference_type;
			using value_type = std::remove_const_t<TIteratorEntity>;
			using pointer = TIteratorEntity*;
			using reference = TIteratorEntity&;
			using iterator_category = std::bidirectional_iterator_tag;

		public:
			/// Creates an iterator around \a current.
			explicit iterator(TIterator current) : m_current(current)
			{}

		public:
			/// Returns \c true if this iterator and \a rhs are equal.
			bool operator==(const iterator& rhs) const {
				return m_current == rhs.m_current;
			}

			/// Returns \c true if this iterator and \a rhs are not equal.
			bool operator!=(const iterator& rhs) const {
				return !(*this == rhs);
			}

		public:
			/// Advances the iterator to the next position.
			iterator& operator++() {
				++m_current;
				return *this;
			}

			/// Advances the iterator to the next position.
			iterator operator++(int) {
				auto copy = *this;
				++m_current;
				return copy;
			}

			/// Advances the iterator to the previous position.
			iterator& operator--() {
				--m_current;
				return *this;
			}

			/// Advances the iterator to the previous position.
			iterator operator--(int) {
				auto copy = *this;
				--m_current;
				return copy;
			}

		public:
			/// Gets a reference to the current entity.
			reference operator*() const {
				return *this->operator->();
			}

			/// Gets a pointer to the current entity.
			pointer operator->() const {
				return *m_current;
			}

			/// Gets a reference to the current entity.
			reference operator*() {
				return *this->operator->();
			}

			/// Gets a pointer to the current entity.
			pointer operator->() {
				return *m_current;
			}

		private:
			TIterator m_current;
		};

	public:
		/// Makes a const iterator initialized with \a current.
		template<typename TIterator>
		static auto make_const_iterator(TIterator current) {
			return iterator<TIterator, const value_type>(current);
		}

		/// Makes an iterator initialized with \a current.
		template<typename TIterator>
		static auto make_iterator(TIterator current) {
			return iterator<TIterator, value_type>(current);
		}
	};

	// endregion

	// region EntityRange

	/// Represents a range of entities.
	template<typename TEntity>
	class EntityRange
			: public EntityRangeFactoryMixin<TEntity>
			, public utils::MoveOnly {
	public:
		using value_type = TEntity;

	public:
		/// Creates an empty entity range.
		EntityRange()
		{}

	private:
		explicit EntityRange(EntityRangeStorage<TEntity>&& storage) : m_storage(std::move(storage))
		{}

	public:
		/// Gets a value indicating whether or not this range is empty.
		bool empty() const {
			return m_storage.subRange().empty();
		}

		/// Gets the size of this range.
		size_t size() const {
			return m_storage.subRange().size();
		}

		/// Gets the total size of the range in bytes.
		size_t totalSize() const {
			return m_storage.subRange().totalSize();
		}

	public:
		/// Gets a const iterator that represents the first entity.
		auto cbegin() const {
			return EntityRangeIteratorFactory<TEntity>::make_const_iterator(m_storage.subRange().entities().cbegin());
		}

		/// Gets a const iterator that represents one past the last entity.
		auto cend() const {
			return EntityRangeIteratorFactory<TEntity>::make_const_iterator(m_storage.subRange().entities().cend());
		}

		/// Gets a const iterator that represents the first entity.
		auto begin() const {
			return cbegin();
		}

		/// Gets a const iterator that represents one past the last entity.
		auto end() const {
			return cend();
		}

		/// Gets an iterator that represents the first entity.
		auto begin() {
			return EntityRangeIteratorFactory<TEntity>::make_iterator(m_storage.subRange().entities().begin());
		}

		/// Gets an iterator that represents one past the last entity.
		auto end() {
			return EntityRangeIteratorFactory<TEntity>::make_iterator(m_storage.subRange().entities().end());
		}

	public:
		/// Gets a const pointer to the start of the data range.
		/// \note This will throw if not supported.
		const auto* data() const {
			m_storage.requireContiguousData();
			return empty() ? nullptr : &*cbegin();
		}

		/// Gets a pointer to the start of the data range.
		/// \note This will throw if not supported.
		auto* data() {
			m_storage.requireContiguousData();
			return empty() ? nullptr : &*begin();
		}

	public:
		/// Creates an entity range by making a copy of an existing range \a rhs.
		static EntityRange CopyRange(const EntityRange& rhs) {
			return EntityRange(EntityRangeStorage<TEntity>(rhs.m_storage.copySubRange()));
		}

		/// Extracts a vector of entities from \a range such that each entity will extend the
		/// lifetime of the owning range.
		static std::vector<std::shared_ptr<TEntity>> ExtractEntitiesFromRange(EntityRange&& range) {
			return range.m_storage.detachSubRangeEntities();
		}

	private:
		friend class EntityRangeFactoryMixin<TEntity>;

	private:
		EntityRangeStorage<TEntity> m_storage;
	};

	// endregion

	// region utils

	/// Compares two entity ranges (\a lhs and \a rhs) and returns the index of the first non-equal element.
	template<typename TEntity>
	size_t FindFirstDifferenceIndex(const EntityRange<TEntity>& lhs, const EntityRange<TEntity>& rhs) {
		auto lhsIter = lhs.cbegin();
		auto rhsIter = rhs.cbegin();
		size_t index = 0;

		while (lhs.cend() != lhsIter && rhs.cend() != rhsIter) {
			if (*lhsIter != *rhsIter)
				break;

			++index;
			++lhsIter;
			++rhsIter;
		}

		return index;
	}

	// endregion
}}
