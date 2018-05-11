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

#include "RocksDatabase.h"
#include "RocksInclude.h"
#include "catapult/exceptions.h"
#include "catapult/utils/HexFormatter.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace cache {

	struct RdbDataIterator::Impl {
		rocksdb::PinnableSlice Result;
	};

	RdbDataIterator::RdbDataIterator(StorageStrategy storageStrategy)
			: m_pImpl(StorageStrategy::Allocate == storageStrategy ? std::make_shared<Impl>() : nullptr)
			, m_isFound(false)
	{}

	RdbDataIterator::RdbDataIterator() : RdbDataIterator(StorageStrategy::Allocate)
	{}

	RdbDataIterator::~RdbDataIterator() = default;

	RdbDataIterator::RdbDataIterator(const RdbDataIterator&) = default;

	RdbDataIterator RdbDataIterator::End() {
		return RdbDataIterator(StorageStrategy::Do_Not_Allocate);
	}

	bool RdbDataIterator::operator==(const RdbDataIterator& rhs) const {
		return m_isFound == rhs.m_isFound;
	}

	bool RdbDataIterator::operator!=(const RdbDataIterator& rhs) const {
		return !(*this == rhs);
	}

	rocksdb::PinnableSlice& RdbDataIterator::storage() const {
		return m_pImpl->Result;
	}

	void RdbDataIterator::setFound(bool found) {
		m_isFound = found;
	}

	RawBuffer RdbDataIterator::buffer() const {
		return { reinterpret_cast<const uint8_t*>(storage().data()), storage().size() };
	}

	RocksDatabase::RocksDatabase(const std::string& dbDir, const std::vector<std::string>& columnFamilyNames) : m_dbDir(dbDir) {
		boost::system::error_code ec;
		boost::filesystem::create_directories(dbDir, ec);

		rocksdb::DB* pDb;
		rocksdb::Options dbOptions;
		dbOptions.create_if_missing = true;
		dbOptions.create_missing_column_families = true;

		std::vector<rocksdb::ColumnFamilyDescriptor> columnFamilies;
		columnFamilies.push_back(rocksdb::ColumnFamilyDescriptor("default", rocksdb::ColumnFamilyOptions()));
		for (const auto& columnFamilyName : columnFamilyNames)
			columnFamilies.push_back(rocksdb::ColumnFamilyDescriptor(columnFamilyName, rocksdb::ColumnFamilyOptions()));

		auto status = rocksdb::DB::Open(dbOptions, m_dbDir, columnFamilies, &m_handles, &pDb);
		m_pDb.reset(pDb);
		if (!status.ok())
			CATAPULT_THROW_RUNTIME_ERROR_2("couldn't open database", dbDir, status.ToString());
	}

	RocksDatabase::~RocksDatabase() {
		for (auto* pHandle : m_handles)
			m_pDb->DestroyColumnFamilyHandle(pHandle);
	}

	namespace {
		[[noreturn]]
		void ThrowError(const char* message, size_t columnId, const rocksdb::Slice& key) {
			CATAPULT_THROW_RUNTIME_ERROR_2(message, columnId, utils::HexFormat(key.data(), key.data() + key.size()));
		}
	}

	void RocksDatabase::get(size_t columnId, const rocksdb::Slice& key, RdbDataIterator& result) {
		auto status = m_pDb->Get(rocksdb::ReadOptions(), m_handles[columnId], key, &result.storage());
		result.setFound(status.ok());

		if (status.ok())
			return;

		if (!status.IsNotFound())
			ThrowError("could not retrieve value (column, key)", columnId, key);
	}

	void RocksDatabase::put(size_t columnId, const rocksdb::Slice& key, const std::string& value) {
		auto status = m_pDb->Put(rocksdb::WriteOptions(), m_handles[columnId], key, value);

		if (!status.ok())
			ThrowError("could not store value in db (column, key)", columnId, key);
	}

	void RocksDatabase::del(size_t columnId, const rocksdb::Slice& key) {
		// note: using SingleDelete can result in undefined result if value has ever been overwritten
		// that can't be guaranteed, so Delete is used instead
		auto status = m_pDb->Delete(rocksdb::WriteOptions(), m_handles[columnId], key);

		if (!status.ok())
			ThrowError("could not remove value from db (column, key)", columnId, key);
	}
}}
