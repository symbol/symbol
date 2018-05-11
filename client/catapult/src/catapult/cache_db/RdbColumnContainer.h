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
#include "catapult/types.h"

namespace catapult {
	namespace cache {
		class RdbDataIterator;
		class RocksDatabase;
	}
}

namespace catapult { namespace cache {

	/// RocksDb-backed container adapter.
	class RdbColumnContainer {
	public:
		/// Creates an adapter around \a database and \a columnId.
		RdbColumnContainer(RocksDatabase& database, size_t columnId);

	public:
		/// Returns size of the column.
		size_t size() const;

		/// Sets size of the column to \a newSize.
		void saveSize(size_t newSize);

		/// Finds element with \a key, storing result in \a iterator.
		void find(const RawBuffer& key, RdbDataIterator& iterator);

		/// Inserts element with \a key and \a value.
		void insert(const RawBuffer& key, const std::string& value);

		/// Removes element with \a key.
		void remove(const RawBuffer& key);

	private:
		RocksDatabase& m_database;
		size_t m_columnId;
		size_t m_size;
	};
}}
