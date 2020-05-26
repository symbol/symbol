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

#include "CompactMosaicMap.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace state {

	// region FirstLevelStorage

	bool CompactMosaicMap::FirstLevelStorage::hasValue() const {
		return MosaicId() != Value.ConstMosaic.first;
	}

	bool CompactMosaicMap::FirstLevelStorage::hasArray() const {
		return !!pNextStorage;
	}

	bool CompactMosaicMap::FirstLevelStorage::hasMap() const {
		return !!pNextStorage && !!pNextStorage->pMapStorage;
	}

	uint8_t& CompactMosaicMap::FirstLevelStorage::arraySize() const {
		return pNextStorage->ArraySize;
	}

	CompactMosaicMap::MosaicArray& CompactMosaicMap::FirstLevelStorage::array() const {
		return pNextStorage->ArrayStorage;
	}

	CompactMosaicMap::MosaicMap& CompactMosaicMap::FirstLevelStorage::map() const {
		return *pNextStorage->pMapStorage;
	}

	// endregion

	// region basic_iterator

	CompactMosaicMap::basic_iterator::basic_iterator(FirstLevelStorage& storage, Stage stage)
			: m_storage(storage)
			, m_stage(stage)
			, m_pCurrent(nullptr) {
		advance();
	}

	CompactMosaicMap::basic_iterator::basic_iterator(FirstLevelStorage& storage, const MosaicLocation& location)
			: m_storage(storage)
			, m_stage(static_cast<Stage>(utils::to_underlying_type(location.Source) + 1))
			, m_arrayIndex(location.ArrayIndex)
			, m_mapIterator(location.MapIterator) {
		switch (location.Source) {
		case MosaicSource::Value:
			setValueMosaic();
			break;

		case MosaicSource::Array:
			setArrayMosaic();
			break;

		case MosaicSource::Map:
			setMapMosaic();
			break;
		}
	}

	bool CompactMosaicMap::basic_iterator::operator==(const basic_iterator& rhs) const {
		return &m_storage == &rhs.m_storage && m_pCurrent == rhs.m_pCurrent;
	}

	bool CompactMosaicMap::basic_iterator::operator!=(const basic_iterator& rhs) const {
		return !(*this == rhs);
	}

	CompactMosaicMap::basic_iterator& CompactMosaicMap::basic_iterator::operator++() {
		if (isEnd())
			CATAPULT_THROW_OUT_OF_RANGE("cannot advance iterator beyond end");

		advance();
		return *this;
	}

	CompactMosaicMap::basic_iterator CompactMosaicMap::basic_iterator::operator++(int) {
		auto copy = *this;
		++*this;
		return copy;
	}

	CompactMosaicMap::Mosaic& CompactMosaicMap::basic_iterator::current() const {
		if (isEnd())
			CATAPULT_THROW_OUT_OF_RANGE("cannot dereference at end");

		return *m_pCurrent;
	}

	void CompactMosaicMap::basic_iterator::advance() {
		switch (m_stage) {
		case Stage::Start:
			return m_storage.hasValue() ? setValueMosaic() : setEnd();

		case Stage::Value:
			m_arrayIndex = 0;
			return m_storage.hasArray() ? setArrayMosaic() : setEnd();

		case Stage::Array:
			if (++m_arrayIndex != m_storage.arraySize())
				return setArrayMosaic();

			if (!m_storage.hasMap())
				return setEnd();

			m_mapIterator = m_storage.map().begin();
			return setMapMosaic();

		case Stage::Map:
			return m_storage.map().cend() != ++m_mapIterator ? setMapMosaic() : setEnd();

		case Stage::End:
			return;
		}
	}

	void CompactMosaicMap::basic_iterator::setValueMosaic() {
		m_stage = Stage::Value;
		m_pCurrent = &m_storage.Value.ConstMosaic;
	}

	void CompactMosaicMap::basic_iterator::setArrayMosaic() {
		m_stage = Stage::Array;
		m_pCurrent = &m_storage.array()[m_arrayIndex].ConstMosaic;
	}

	void CompactMosaicMap::basic_iterator::setMapMosaic() {
		m_stage = Stage::Map;
		m_pCurrent = &*m_mapIterator;
	}

	void CompactMosaicMap::basic_iterator::setEnd() {
		m_stage = Stage::End;
		m_pCurrent = nullptr;
	}

	bool CompactMosaicMap::basic_iterator::isEnd() const {
		return Stage::End == m_stage;
	}

	// endregion

	CompactMosaicMap::const_iterator CompactMosaicMap::begin() const {
		return const_iterator(const_cast<FirstLevelStorage&>(m_storage), const_iterator::Stage::Start);
	}

	CompactMosaicMap::const_iterator CompactMosaicMap::end() const {
		return const_iterator(const_cast<FirstLevelStorage&>(m_storage), const_iterator::Stage::End);
	}

	CompactMosaicMap::iterator CompactMosaicMap::begin() {
		return iterator(m_storage, const_iterator::Stage::Start);
	}

	CompactMosaicMap::iterator CompactMosaicMap::end() {
		return iterator(m_storage, const_iterator::Stage::End);
	}

	bool CompactMosaicMap::empty() const {
		return !m_storage.hasValue();
	}

	size_t CompactMosaicMap::size() const {
		if (empty())
			return 0;

		size_t count = 1;
		if (m_storage.hasArray()) {
			count += m_storage.arraySize();
			if (m_storage.hasMap())
				count += m_storage.map().size();
		}

		return count;
	}

	CompactMosaicMap::const_iterator CompactMosaicMap::find(MosaicId id) const {
		MosaicLocation location;
		return find(id, location) ? const_iterator(const_cast<FirstLevelStorage&>(m_storage), location) : end();
	}

	CompactMosaicMap::iterator CompactMosaicMap::find(MosaicId id) {
		MosaicLocation location;
		return find(id, location) ? iterator(m_storage, location) : end();
	}

	namespace {
		bool IsLessThan(MosaicId optimizedMosaicId, MosaicId lhs, MosaicId rhs) {
			// customize so that optimized mosaic id is smallest
			if (lhs == rhs)
				return false;

			if (optimizedMosaicId == lhs || optimizedMosaicId == rhs)
				return optimizedMosaicId == lhs;

			return lhs < rhs;
		}
	}

	void CompactMosaicMap::insert(const Mosaic& pair) {
		if (MosaicId() == pair.first)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot insert reserved mosaic", pair.first);

		if (end() != utils::as_const(*this).find(pair.first))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot insert mosaic already in map", pair.first);

		if (empty()) {
			m_storage.Value.Mosaic = pair;
			return;
		}

		// use insertion sort to insert into array
		if (!m_storage.hasArray())
			m_storage.pNextStorage = std::make_unique<SecondLevelStorage>();

		if (IsLessThan(m_optimizedMosaicId, pair.first, m_storage.Value.ConstMosaic.first)) {
			insertIntoArray(0, m_storage.Value.ConstMosaic);
			m_storage.Value.Mosaic = pair;
			return;
		}

		auto arrayIndex = 0u;
		for (; arrayIndex < m_storage.arraySize(); ++arrayIndex) {
			if (pair.first < m_storage.array()[arrayIndex].ConstMosaic.first)
				break;
		}

		if (arrayIndex < Array_Size) {
			insertIntoArray(arrayIndex, pair);
			return;
		}

		// if all array values are smaller, insert into map
		insertIntoMap(pair);
	}

	void CompactMosaicMap::erase(MosaicId id) {
		MosaicLocation location;
		if (!find(id, location))
			return;

		switch (location.Source) {
		case MosaicSource::Value:
			if (m_storage.hasArray()) {
				m_storage.Value.Mosaic = m_storage.array()[0].ConstMosaic;
				eraseFromArray(0);
			} else {
				m_storage.Value.Mosaic = MutableMosaic();
			}
			break;

		case MosaicSource::Array:
			eraseFromArray(location.ArrayIndex);
			break;

		case MosaicSource::Map:
			m_storage.map().erase(id);
			break;
		}

		compact();
	}

	namespace {
		void reinsert(CompactMosaicMap& map, MosaicId id) {
			auto iter = map.find(id);
			if (map.end() == iter)
				return;

			// copy and reinsert the matching mosaic
			auto mosaicCopy = *iter;
			map.erase(mosaicCopy.first);
			map.insert(mosaicCopy);
		}
	}

	void CompactMosaicMap::optimize(MosaicId id) {
		if (id == m_optimizedMosaicId)
			return;

		auto previousOptimizedMosaicId = m_optimizedMosaicId;
		m_optimizedMosaicId = id;

		reinsert(*this, previousOptimizedMosaicId);
		reinsert(*this, m_optimizedMosaicId);
	}

	bool CompactMosaicMap::find(MosaicId id, MosaicLocation& location) const {
		if (empty())
			return false;

		if (id == m_storage.Value.ConstMosaic.first) {
			location.Source = MosaicSource::Value;
			return true;
		}

		if (m_storage.hasArray()) {
			for (auto i = 0u; i < m_storage.arraySize(); ++i) {
				if (id == m_storage.array()[i].ConstMosaic.first) {
					location.Source = MosaicSource::Array;
					location.ArrayIndex = i;
					return true;
				}
			}
		}

		if (m_storage.hasMap()) {
			auto iter = m_storage.map().find(id);
			if (m_storage.map().end() != iter) {
				location.Source = MosaicSource::Map;
				location.MapIterator = iter;
				return true;
			}
		}

		return false;
	}

	void CompactMosaicMap::insertIntoArray(size_t index, const Mosaic& pair) {
		// move the last array value into the map
		if (Array_Size == m_storage.arraySize())
			insertIntoMap(m_storage.array().back().ConstMosaic);
		else
			++m_storage.arraySize();

		for (auto i = m_storage.arraySize(); i > index + 1; --i)
			m_storage.array()[i - 1].Mosaic = m_storage.array()[i - 2].ConstMosaic;

		m_storage.array()[index].Mosaic = pair;
	}

	void CompactMosaicMap::insertIntoMap(const Mosaic& pair) {
		if (!m_storage.hasMap())
			m_storage.pNextStorage->pMapStorage = std::make_unique<MosaicMap>();

		m_storage.pNextStorage->pMapStorage->insert(pair);
	}

	void CompactMosaicMap::eraseFromArray(size_t index) {
		for (auto i = index; i < m_storage.arraySize() - 1u; ++i)
			m_storage.array()[i].Mosaic = m_storage.array()[i + 1].ConstMosaic;

		--m_storage.arraySize();
	}

	void CompactMosaicMap::compact() {
		if (!m_storage.hasArray())
			return;

		if (m_storage.hasMap()) {
			// compact into array
			while (m_storage.arraySize() < Array_Size && !m_storage.map().empty()) {
				m_storage.array()[m_storage.arraySize()++].Mosaic = *m_storage.map().cbegin();
				m_storage.map().erase(m_storage.map().cbegin());
			}

			if (m_storage.map().empty())
				m_storage.pNextStorage->pMapStorage.reset();
		}

		if (0 == m_storage.arraySize())
			m_storage.pNextStorage.reset();
	}
}}
