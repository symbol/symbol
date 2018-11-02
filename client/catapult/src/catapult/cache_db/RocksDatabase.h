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
#include "RocksPruningFilter.h"
#include "catapult/utils/FileSize.h"
#include "catapult/types.h"
#include <memory>
#include <string>
#include <vector>

namespace rocksdb {
	class ColumnFamilyHandle;
	class DB;
	class PinnableSlice;
	class Slice;
	class WriteBatch;
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

	public:
		/// Move constructor.
		RdbDataIterator(RdbDataIterator&&);

		/// Move assignment operator.
		RdbDataIterator& operator=(RdbDataIterator&&);

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

	/// RocksDb settings.
	struct RocksDatabaseSettings {
	public:
		/// Creates default database settings.
		RocksDatabaseSettings();

		/// Creates database settings around \a databaseDirectory, column names (\a columnFamilyNames),
		/// maximum size of saved batch (\a maxDatabaseWriteBatchSize) and \a pruningMode.
		RocksDatabaseSettings(
				const std::string& databaseDirectory,
				const std::vector<std::string>& columnFamilyNames,
				utils::FileSize maxDatabaseWriteBatchSize,
				FilterPruningMode pruningMode);

	public:
		/// Database directory.
		const std::string DatabaseDirectory;

		/// Names of database columns.
		const std::vector<std::string> ColumnFamilyNames;

		/// Maximum size of database write batch.
		const utils::FileSize MaxDatabaseWriteBatchSize;

		/// Database pruning mode.
		const FilterPruningMode PruningMode;
	};

	/// RocksDb-backed database.
	class RocksDatabase {
	public:
		/// Creates an empty database.
		RocksDatabase();

		/// Creates database around \a settings.
		explicit RocksDatabase(const RocksDatabaseSettings& settings);

		/// Destroys database.
		~RocksDatabase();

	public:
		/// Gets the database column family names.
		const std::vector<std::string>& columnFamilyNames() const;

		/// Returns \c true if pruning is enabled.
		bool canPrune() const;

	public:
		/// Gets \a key from \a columnId returning data in \a result.
		void get(size_t columnId, const rocksdb::Slice& key, RdbDataIterator& result);

		/// Puts \a value with \a key in \a columnId.
		void put(size_t columnId, const rocksdb::Slice& key, const std::string& value);

		/// Deletes \a key from \a columnId.
		void del(size_t columnId, const rocksdb::Slice& key);

		/// Prunes elements from \a columnId below \a boundary. Returns number of pruned elements.
		size_t prune(size_t columnId, uint64_t boundary);

		/// Finalize batched operations.
		void flush();

	private:
		void saveIfBatchFull();

	private:
		const RocksDatabaseSettings m_settings;
		RocksPruningFilter m_pruningFilter;
		std::unique_ptr<rocksdb::WriteBatch> m_pWriteBatch;

		std::unique_ptr<rocksdb::DB> m_pDb;
		std::vector<rocksdb::ColumnFamilyHandle*> m_handles;
	};
}}
