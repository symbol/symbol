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
#include "catapult/utils/MemoryUtils.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/exceptions.h"
#include <memory>
#include <vector>

namespace catapult { namespace model {

	/// Represents a range of entities.
	template<typename TEntity>
	class EntityRange : public utils::MoveOnly {
	public:
		using value_type = TEntity;

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

			SingleBufferRange(size_t dataSize, const std::vector<size_t>& offsets)
					: SubRange(dataSize)
					, m_buffer(dataSize) {
				for (auto offset : offsets)
					SubRange::entities().push_back(reinterpret_cast<TEntity*>(&m_buffer[offset]));
			}

			SingleBufferRange(const uint8_t* pData, size_t dataSize, const std::vector<size_t>& offsets)
					: SingleBufferRange(dataSize, offsets) {
				utils::memcpy_cond(m_buffer.data(), pData, dataSize);
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
				return SingleBufferRange(data(), SubRange::totalSize(), generateOffsets());
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
				return SingleBufferRange(reinterpret_cast<const uint8_t*>(m_pSingleEntity.get()), SubRange::totalSize(), { 0 });
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

			explicit MultiBufferRange(std::vector<EntityRange>&& ranges)
					: SubRange(CalculateTotalSize(ranges))
					, m_ranges(std::move(ranges)) {
				for (auto& range : m_ranges)
					for (auto& entity : range)
						SubRange::entities().push_back(&entity);
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
				std::vector<EntityRange> copyRanges;
				for (const auto& range : m_ranges)
					copyRanges.push_back(range.copySubRange());

				return MultiBufferRange(std::move(copyRanges));
			}

		private:
			static size_t CalculateTotalSize(const std::vector<EntityRange>& ranges) {
				size_t totalSize = 0;
				for (const auto& range : ranges)
					totalSize += range.totalSize();

				return totalSize;
			}

		private:
			std::vector<EntityRange> m_ranges;
		};

		// endregion

	public:
		/// Creates an empty entity range.
		EntityRange()
		{}

	private:
		explicit EntityRange(SingleBufferRange&& subRange)
				: m_singleBufferRange(std::move(subRange))
		{}

		explicit EntityRange(SingleEntityRange&& subRange)
				: m_singleEntityRange(std::move(subRange))
		{}

		explicit EntityRange(MultiBufferRange&& subRange)
				: m_multiBufferRange(std::move(subRange))
		{}

	public:
		/// Creates an uninitialized entity range of contiguous memory around \a numElements fixed size elements.
		/// \a ppRangeData is set to point to the range memory.
		static EntityRange PrepareFixed(size_t numElements, uint8_t** ppRangeData = nullptr) {
			std::vector<size_t> offsets(numElements);
			for (auto i = 0u; i < numElements; ++i)
				offsets[i] = i * sizeof(TEntity);

			auto range = EntityRange(SingleBufferRange(numElements * sizeof(TEntity), offsets));
			if (ppRangeData)
				*ppRangeData = range.m_singleBufferRange.data();

			return range;
		}

		/// Creates an entity range around \a numElements fixed size elements pointed to by \a pData.
		static EntityRange CopyFixed(const uint8_t* pData, size_t numElements) {
			uint8_t* pRangeData;
			auto range = PrepareFixed(numElements, &pRangeData);
			utils::memcpy_cond(pRangeData, pData, range.totalSize());
			return range;
		}

		/// Creates an entity range around the data pointed to by \a pData with size \a dataSize and \a offsets
		/// container that contains values indicating the starting position of all entities in the data.
		static EntityRange CopyVariable(const uint8_t* pData, size_t dataSize, const std::vector<size_t>& offsets) {
			return EntityRange(SingleBufferRange(pData, dataSize, offsets));
		}

		/// Creates an entity range around a single entity (\a pEntity).
		static EntityRange FromEntity(std::unique_ptr<TEntity>&& pEntity) {
			return EntityRange(SingleEntityRange(std::move(pEntity)));
		}

		/// Merges all \a ranges into a single range.
		static EntityRange MergeRanges(std::vector<EntityRange>&& ranges) {
			return EntityRange(MultiBufferRange(std::move(ranges)));
		}

	public:
		/// Gets a value indicating whether or not this range is empty.
		bool empty() const {
			return subRange().empty();
		}

		/// Gets the size of this range.
		size_t size() const {
			return subRange().size();
		}

		/// Gets the total size of the range in bytes.
		size_t totalSize() const {
			return subRange().totalSize();
		}

	// region iterators
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

	private:
		template<typename TIterator>
		static auto make_const_iterator(TIterator current) {
			return iterator<TIterator, const value_type>(current);
		}

		template<typename TIterator>
		static auto make_iterator(TIterator current) {
			return iterator<TIterator, value_type>(current);
		}

	public:
		/// Gets a const iterator that represents the first entity.
		auto cbegin() const {
			return make_const_iterator(subRange().entities().cbegin());
		}

		/// Gets a const iterator that represents one past the last entity.
		auto cend() const {
			return make_const_iterator(subRange().entities().cend());
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
			return make_iterator(subRange().entities().begin());
		}

		/// Gets an iterator that represents one past the last entity.
		auto end() {
			return make_iterator(subRange().entities().end());
		}

	// endregion

	public:
		/// Gets a const pointer to the start of the data range.
		/// \note This will throw if not supported.
		const auto* data() const {
			requireContiguousData();
			return empty() ? nullptr : &*cbegin();
		}

		/// Gets a pointer to the start of the data range.
		/// \note This will throw if not supported.
		auto* data() {
			requireContiguousData();
			return empty() ? nullptr : &*begin();
		}

	private:
		void requireContiguousData() const {
			if (!m_multiBufferRange.empty())
				CATAPULT_THROW_RUNTIME_ERROR("data is not accessible when range is composed of non-contiguous data");
		}

	public:
		/// Creates an entity range by making a copy of an existing range \a rhs.
		static EntityRange CopyRange(const EntityRange& rhs) {
			return rhs.copySubRange();
		}

		/// Extracts a vector of entities from \a range such that each entity will extend the
		/// lifetime of the owning range.
		static std::vector<std::shared_ptr<TEntity>> ExtractEntitiesFromRange(EntityRange&& range) {
			return range.detachSubRangeEntities();
		}

	private:
		const SubRange& subRange() const {
			return const_cast<EntityRange&>(*this).subRange();
		}

		SubRange& subRange() {
			SubRange* pSubRange;
			activeSubRangeAction([&pSubRange](auto& subRange) { pSubRange = &subRange; });
			return *pSubRange;
		}

		auto copySubRange() const {
			return activeSubRangeAction([](const auto& subRange) { return EntityRange(subRange.copy()); });
		}

		auto detachSubRangeEntities() {
			return activeSubRangeAction([](auto& subRange) {
				auto entities = subRange.detachEntities();
				subRange.reset();
				return entities;
			});
		}

		template<typename TFunc>
		auto activeSubRangeAction(TFunc func) const {
			return const_cast<EntityRange&>(*this).activeSubRangeAction(func);
		}

		template<typename TFunc>
		auto activeSubRangeAction(TFunc func) {
			if (!m_singleEntityRange.empty())
				return func(m_singleEntityRange);

			if (!m_multiBufferRange.empty())
				return func(m_multiBufferRange);

			return func(m_singleBufferRange);
		}

	private:
		friend class MultiBufferRange;

	private:
		SingleBufferRange m_singleBufferRange;
		SingleEntityRange m_singleEntityRange;
		MultiBufferRange m_multiBufferRange;
	};

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
}}
