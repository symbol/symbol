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

#include "RdbColumnContainer.h"
#include "RocksDatabase.h"
#include "RocksInclude.h"

namespace catapult { namespace cache {

	namespace {
		auto ToSlice(const RawBuffer& key) {
			return rocksdb::Slice(reinterpret_cast<const char*>(key.pData), key.Size);
		}
	}

	RdbColumnContainer::RdbColumnContainer(RocksDatabase& database, size_t columnId)
			: m_database(database)
			, m_columnId(columnId) {
		RdbDataIterator iter;
		m_database.get(m_columnId, "size", iter);
		m_size = RdbDataIterator::End() == iter
				? 0
				: static_cast<size_t>(*reinterpret_cast<const uint64_t*>(iter.storage().data()));
	}

	size_t RdbColumnContainer::size() const {
		return m_size;
	}

	void RdbColumnContainer::saveSize(size_t newSize) {
		std::string strSize(sizeof(uint64_t), 0);
		*reinterpret_cast<uint64_t*>(&strSize[0]) = static_cast<uint64_t>(newSize);
		m_database.put(m_columnId, "size", strSize);
		m_size = newSize;
	}

	void RdbColumnContainer::find(const RawBuffer& key, RdbDataIterator& iterator) {
		m_database.get(m_columnId, ToSlice(key), iterator);
	}

	void RdbColumnContainer::insert(const RawBuffer& key, const std::string& value) {
		m_database.put(m_columnId, ToSlice(key), value);
	}

	void RdbColumnContainer::remove(const RawBuffer& key) {
		m_database.del(m_columnId, ToSlice(key));
	}
}}
