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

#include "RdbColumnContainer.h"
#include "RocksDatabase.h"
#include "RocksInclude.h"

namespace catapult { namespace cache {

	namespace {
		auto ToSlice(const RawBuffer& key) {
			return rocksdb::Slice(reinterpret_cast<const char*>(key.pData), key.Size);
		}

		void VerifyName(const std::string& propertyName) {
			if (propertyName.size() >= Special_Key_Max_Length)
				CATAPULT_THROW_INVALID_ARGUMENT_1("property name too long", propertyName);
		}
	}

	RdbColumnContainer::RdbColumnContainer(RocksDatabase& database, size_t columnId)
			: m_database(database)
			, m_columnId(columnId) {
		uint64_t size = 0;
		load("size", [&size](const char* buffer) {
			if (!buffer)
				return;

			size = reinterpret_cast<const uint64_t&>(*buffer);
		});

		m_size = static_cast<size_t>(size);
	}

	void RdbColumnContainer::save(const std::string& propertyName, const std::string& strValue) {
		VerifyName(propertyName);
		m_database.put(m_columnId, propertyName, strValue);
	}

	void RdbColumnContainer::load(const std::string& propertyName, const consumer<const char*>& sink) const {
		VerifyName(propertyName);
		RdbDataIterator iter;
		m_database.get(m_columnId, propertyName, iter);
		sink(RdbDataIterator::End() == iter ? nullptr : iter.storage().data());
	}

	size_t RdbColumnContainer::size() const {
		return m_size;
	}

	void RdbColumnContainer::setSize(size_t newSize) {
		setProp("size", static_cast<uint64_t>(newSize));
		m_size = newSize;
	}

	void RdbColumnContainer::find(const RawBuffer& key, RdbDataIterator& iterator) const {
		m_database.get(m_columnId, ToSlice(key), iterator);
	}

	void RdbColumnContainer::insert(const RawBuffer& key, const std::string& value) {
		m_database.put(m_columnId, ToSlice(key), value);
	}

	void RdbColumnContainer::remove(const RawBuffer& key) {
		m_database.del(m_columnId, ToSlice(key));
	}

	size_t RdbColumnContainer::prune(uint64_t pruningBoundary) {
		return m_database.prune(m_columnId, pruningBoundary);
	}
}}
