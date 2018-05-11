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
#include <memory>
#include <string>
#include <vector>

namespace rocksdb {
	class ColumnFamilyHandle;
	class DB;
	class PinnableSlice;
	class Slice;
}

namespace catapult { namespace cache {

	/// Iterator adapter, allowing existence check on keys and data retrieval.
	class RdbDataIterator {
	private:
		enum class StorageStrategy { Allocate, Do_Not_Allocate };

		explicit RdbDataIterator(StorageStrategy storageStrategy);

	public:
		/// Creates an iterator.
		RdbDataIterator();

		/// Destroys an iterator.
		~RdbDataIterator();

		/// Copy constructor.
		RdbDataIterator(const RdbDataIterator&);

	public:
		/// Iterator representing no match.
		static RdbDataIterator End();

	public:
		/// Returns \c true if this iterator and \a rhs are equal.
		bool operator==(const RdbDataIterator& rhs) const;

		/// Returns \c true if this iterator and \a rhs are not equal.
		bool operator!=(const RdbDataIterator& rhs) const;

	public:
		/// Returns storage associated with iterator.
		rocksdb::PinnableSlice& storage() const;

		/// Sets flag to \a found indicating that iterator contains data.
		void setFound(bool found);

		/// Returns storage as raw buffer.
		RawBuffer buffer() const;

	private:
		struct Impl;
		std::shared_ptr<Impl> m_pImpl;
		bool m_isFound;
	};

	/// RocksDb-backed database.
	class RocksDatabase {
	public:
		/// Creates database in \a dbDir with 'default' column and additional columns (\a columnFamilyNames).
		RocksDatabase(const std::string& dbDir, const std::vector<std::string>& columnFamilyNames);

		/// Destroys database.
		~RocksDatabase();

	public:
		/// Gets \a key from \a columnId returning data in \a result.
		void get(size_t columnId, const rocksdb::Slice& key, RdbDataIterator& result);

		/// Puts \a value with \a key in \a columnId.
		void put(size_t columnId, const rocksdb::Slice& key, const std::string& value);

		/// Deletes \a key from \a columnId.
		void del(size_t columnId, const rocksdb::Slice& key);

	private:
		std::string m_dbDir;
		std::shared_ptr<rocksdb::DB> m_pDb;
		std::vector<rocksdb::ColumnFamilyHandle*> m_handles;
	};
}}
