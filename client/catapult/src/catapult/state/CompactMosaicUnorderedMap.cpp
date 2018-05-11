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

#include "CompactMosaicUnorderedMap.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace state {

	// region FirstLevelStorage

	bool CompactMosaicUnorderedMap::FirstLevelStorage::hasValue() const {
		return MosaicId() != Value.first;
	}

	bool CompactMosaicUnorderedMap::FirstLevelStorage::hasArray() const {
		return !!pNextStorage;
	}

	bool CompactMosaicUnorderedMap::FirstLevelStorage::hasMap() const {
		return !!pNextStorage && !!pNextStorage->pMapStorage;
	}

	uint8_t& CompactMosaicUnorderedMap::FirstLevelStorage::arraySize() const {
		return pNextStorage->ArraySize;
	}

	CompactMosaicUnorderedMap::MosaicArray& CompactMosaicUnorderedMap::FirstLevelStorage::array() const {
		return pNextStorage->ArrayStorage;
	}

	CompactMosaicUnorderedMap::MosaicMap& CompactMosaicUnorderedMap::FirstLevelStorage::map() const {
		return *pNextStorage->pMapStorage;
	}

	// endregion

	// region basic_iterator

	CompactMosaicUnorderedMap::basic_iterator::basic_iterator(FirstLevelStorage& storage, Stage stage)
			: m_storage(storage)
			, m_stage(stage)
			, m_pCurrent(nullptr) {
		advance();
	}

	CompactMosaicUnorderedMap::basic_iterator::basic_iterator(FirstLevelStorage& storage, const MosaicLocation& location)
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

	bool CompactMosaicUnorderedMap::basic_iterator::operator==(const basic_iterator& rhs) const {
		return &m_storage == &rhs.m_storage && m_pCurrent == rhs.m_pCurrent;
	}

	bool CompactMosaicUnorderedMap::basic_iterator::operator!=(const basic_iterator& rhs) const {
		return !(*this == rhs);
	}

	CompactMosaicUnorderedMap::basic_iterator& CompactMosaicUnorderedMap::basic_iterator::operator++() {
		if (isEnd())
			CATAPULT_THROW_OUT_OF_RANGE("cannot advance iterator beyond end");

		advance();
		return *this;
	}

	CompactMosaicUnorderedMap::basic_iterator CompactMosaicUnorderedMap::basic_iterator::operator++(int) {
		auto copy = *this;
		++*this;
		return copy;
	}

	CompactMosaicUnorderedMap::Mosaic& CompactMosaicUnorderedMap::basic_iterator::current() const {
		if (isEnd())
			CATAPULT_THROW_OUT_OF_RANGE("cannot dereference at end");

		return *m_pCurrent;
	}

	void CompactMosaicUnorderedMap::basic_iterator::advance() {
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

	void CompactMosaicUnorderedMap::basic_iterator::setValueMosaic() {
		m_stage = Stage::Value;
		m_pCurrent = reinterpret_cast<Mosaic*>(&m_storage.Value); // pair<X, Y> => pair<const X, Y>
	}

	void CompactMosaicUnorderedMap::basic_iterator::setArrayMosaic() {
		m_stage = Stage::Array;
		m_pCurrent = reinterpret_cast<Mosaic*>(&m_storage.array()[m_arrayIndex]);
	}

	void CompactMosaicUnorderedMap::basic_iterator::setMapMosaic() {
		m_stage = Stage::Map;
		m_pCurrent = &*m_mapIterator;
	}

	void CompactMosaicUnorderedMap::basic_iterator::setEnd() {
		m_stage = Stage::End;
		m_pCurrent = nullptr;
	}

	bool CompactMosaicUnorderedMap::basic_iterator::isEnd() const {
		return Stage::End == m_stage;
	}

	// endregion

	CompactMosaicUnorderedMap::const_iterator CompactMosaicUnorderedMap::begin() const {
		return const_iterator(const_cast<FirstLevelStorage&>(m_storage), const_iterator::Stage::Start);
	}

	CompactMosaicUnorderedMap::const_iterator CompactMosaicUnorderedMap::end() const {
		return const_iterator(const_cast<FirstLevelStorage&>(m_storage), const_iterator::Stage::End);
	}

	CompactMosaicUnorderedMap::iterator CompactMosaicUnorderedMap::begin() {
		return iterator(m_storage, const_iterator::Stage::Start);
	}

	CompactMosaicUnorderedMap::iterator CompactMosaicUnorderedMap::end() {
		return iterator(m_storage, const_iterator::Stage::End);
	}

	bool CompactMosaicUnorderedMap::empty() const {
		return !m_storage.hasValue();
	}

	size_t CompactMosaicUnorderedMap::size() const {
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

	CompactMosaicUnorderedMap::const_iterator CompactMosaicUnorderedMap::find(MosaicId id) const {
		MosaicLocation location;
		return find(id, location) ? const_iterator(const_cast<FirstLevelStorage&>(m_storage), location) : end();
	}

	CompactMosaicUnorderedMap::iterator CompactMosaicUnorderedMap::find(MosaicId id) {
		MosaicLocation location;
		return find(id, location) ? iterator(m_storage, location) : end();
	}

	void CompactMosaicUnorderedMap::insert(const Mosaic& pair) {
		if (MosaicId() == pair.first)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot insert reserved mosaic", pair.first);

		if (end() != utils::as_const(*this).find(pair.first))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot insert mosaic already in map", pair.first);

		if (empty()) {
			m_storage.Value = pair;
			return;
		}

		if (!m_storage.hasArray())
			m_storage.pNextStorage = std::make_unique<SecondLevelStorage>();

		if (Array_Size != m_storage.arraySize()) {
			m_storage.array()[m_storage.arraySize()++] = pair;
			return;
		}

		if (!m_storage.hasMap())
			m_storage.pNextStorage->pMapStorage = std::make_unique<MosaicMap>();

		m_storage.pNextStorage->pMapStorage->insert(pair);
	}

	void CompactMosaicUnorderedMap::erase(MosaicId id) {
		MosaicLocation location;
		if (!find(id, location))
			return;

		switch (location.Source) {
		case MosaicSource::Value:
			if (m_storage.hasArray())
				m_storage.Value = m_storage.array()[--m_storage.arraySize()];
			else
				m_storage.Value = MutableMosaic();
			break;

		case MosaicSource::Array:
			// replace the erased element with the last element
			m_storage.array()[location.ArrayIndex] = m_storage.array()[--m_storage.arraySize()];
			break;

		case MosaicSource::Map:
			m_storage.map().erase(id);
			break;
		};

		compact();
	}

	bool CompactMosaicUnorderedMap::find(MosaicId id, MosaicLocation& location) const {
		if (empty())
			return false;

		if (id == m_storage.Value.first) {
			location.Source = MosaicSource::Value;
			return true;
		}

		if (m_storage.hasArray()) {
			for (auto i = 0u; i < m_storage.arraySize(); ++i) {
				if (id == m_storage.array()[i].first) {
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

	void CompactMosaicUnorderedMap::compact() {
		if (!m_storage.hasArray())
			return;

		if (m_storage.hasMap()) {
			// compact into array
			while (m_storage.arraySize() < Array_Size && !m_storage.map().empty()) {
				m_storage.array()[m_storage.arraySize()++] = *m_storage.map().cbegin();
				m_storage.map().erase(m_storage.map().cbegin());
			}

			if (m_storage.map().empty())
				m_storage.pNextStorage->pMapStorage.reset();
		}

		if (0 == m_storage.arraySize())
			m_storage.pNextStorage.reset();
	}
}}
