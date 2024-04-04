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

#include "RocksDatabase.h"
#include "RocksInclude.h"
#include "RocksPruningFilter.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/PathUtils.h"
#include "catapult/utils/StackLogger.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	// region RdbDataIterator

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

	RdbDataIterator::RdbDataIterator(RdbDataIterator&&) = default;

	RdbDataIterator& RdbDataIterator::operator=(RdbDataIterator&&) = default;

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

	// endregion

	// region RocksDatabaseSettings

	RocksDatabaseSettings::RocksDatabaseSettings()
			: RocksDatabaseSettings(std::string(), std::vector<std::string>(), FilterPruningMode::Disabled)
	{}

	RocksDatabaseSettings::RocksDatabaseSettings(
			const std::string& databaseDirectory,
			const std::vector<std::string>& columnFamilyNames,
			FilterPruningMode pruningMode)
			: RocksDatabaseSettings(
					databaseDirectory,
					config::NodeConfiguration::CacheDatabaseSubConfiguration(),
					columnFamilyNames,
					pruningMode)
	{}

	RocksDatabaseSettings::RocksDatabaseSettings(
			const std::string& databaseDirectory,
			const config::NodeConfiguration::CacheDatabaseSubConfiguration& databaseConfig,
			const std::vector<std::string>& columnFamilyNames,
			FilterPruningMode pruningMode)
			: DatabaseDirectory(databaseDirectory)
			, DatabaseConfig(databaseConfig)
			, ColumnFamilyNames(columnFamilyNames)
			, PruningMode(pruningMode)
	{}

	// endregion

	// region RocksDatabase

	namespace {
		rocksdb::Options CreateDatabaseOptions(const config::NodeConfiguration::CacheDatabaseSubConfiguration& config) {
			rocksdb::Options dbOptions;
			dbOptions.create_if_missing = true;
			dbOptions.create_missing_column_families = true;

			if (config.MaxOpenFiles > 0)
				dbOptions.max_open_files = static_cast<int>(config.MaxOpenFiles);

			if (config.MaxLogFiles > 0)
				dbOptions.keep_log_file_num = config.MaxLogFiles;

			if (utils::FileSize() != config.MaxLogFileSize)
				dbOptions.max_log_file_size = config.MaxLogFileSize.bytes();

			if (config.MaxBackgroundThreads > 0)
				dbOptions.IncreaseParallelism(static_cast<int>(config.MaxBackgroundThreads));

			if (config.MaxSubcompactionThreads > 0)
				dbOptions.max_subcompactions = config.MaxSubcompactionThreads;

			if (config.EnableStatistics)
				dbOptions.statistics = rocksdb::CreateDBStatistics();

			return dbOptions;
		}

		rocksdb::ColumnFamilyOptions CreateColumnFamilyOptions(
				const config::NodeConfiguration::CacheDatabaseSubConfiguration& config,
				rocksdb::CompactionFilter* pCompactionFilter) {
			rocksdb::ColumnFamilyOptions columnFamilyOptions;
			columnFamilyOptions.compaction_filter = pCompactionFilter;

			if (utils::FileSize() != config.BlockCacheSize)
				columnFamilyOptions.OptimizeForPointLookup(config.BlockCacheSize.megabytes());

			if (utils::FileSize() != config.MemtableMemoryBudget)
				columnFamilyOptions.OptimizeLevelStyleCompaction(config.MemtableMemoryBudget.bytes());

			return columnFamilyOptions;
		}
	}

	RocksDatabase::RocksDatabase() = default;

	RocksDatabase::RocksDatabase(const RocksDatabaseSettings& settings)
			: m_settings(settings)
			, m_pruningFilter(m_settings.PruningMode)
			, m_pWriteBatch(std::make_unique<rocksdb::WriteBatch>()) {
		if (m_settings.ColumnFamilyNames.empty())
			CATAPULT_THROW_INVALID_ARGUMENT("missing column family names");

		config::CatapultDirectory(m_settings.DatabaseDirectory).createAll();

		auto columnFamilyOptions = CreateColumnFamilyOptions(m_settings.DatabaseConfig, m_pruningFilter.compactionFilter());
		std::vector<rocksdb::ColumnFamilyDescriptor> columnFamilies;
		for (const auto& columnFamilyName : m_settings.ColumnFamilyNames)
			columnFamilies.push_back(rocksdb::ColumnFamilyDescriptor(columnFamilyName, columnFamilyOptions));

		rocksdb::DB* pDb;
		auto dbOptions = CreateDatabaseOptions(m_settings.DatabaseConfig);
		auto status = rocksdb::DB::Open(dbOptions, m_settings.DatabaseDirectory, columnFamilies, &m_handles, &pDb);
		m_pDb.reset(pDb);
		if (!status.ok())
			CATAPULT_THROW_RUNTIME_ERROR_2("couldn't open database", m_settings.DatabaseDirectory, status.ToString());
	}

	RocksDatabase::~RocksDatabase() {
		for (auto* pHandle : m_handles)
			m_pDb->DestroyColumnFamilyHandle(pHandle);
	}

	const std::vector<std::string>& RocksDatabase::columnFamilyNames() const {
		return m_settings.ColumnFamilyNames;
	}

	bool RocksDatabase::canPrune() const {
		return FilterPruningMode::Enabled == m_settings.PruningMode;
	}

	namespace {
		[[noreturn]]
		void ThrowError(const std::string& message, const std::string& columnName, const rocksdb::Slice& key) {
			CATAPULT_THROW_RUNTIME_ERROR_2(message.c_str(), columnName, utils::HexFormat(key.data(), key.data() + key.size()));
		}
	}

#define CATAPULT_THROW_DB_KEY_ERROR(message) \
	ThrowError(std::string(message) + " " + status.ToString() + " (column, key)", m_settings.ColumnFamilyNames[columnId], key)

	void RocksDatabase::get(size_t columnId, const rocksdb::Slice& key, RdbDataIterator& result) {
		if (!m_pDb)
			CATAPULT_THROW_INVALID_ARGUMENT("RocksDatabase has not been initialized");

		auto status = m_pDb->Get(rocksdb::ReadOptions(), m_handles[columnId], key, &result.storage());
		result.setFound(status.ok());

		if (status.ok())
			return;

		// note: this is intentional, in case of not found status will be set via `setFound` above
		if (!status.IsNotFound())
			CATAPULT_THROW_DB_KEY_ERROR("could not retrieve value");
	}

	void RocksDatabase::put(size_t columnId, const rocksdb::Slice& key, const std::string& value) {
		if (!m_pDb)
			CATAPULT_THROW_INVALID_ARGUMENT("RocksDatabase has not been initialized");

		auto status = m_pWriteBatch->Put(m_handles[columnId], key, value);
		if (!status.ok())
			CATAPULT_THROW_DB_KEY_ERROR("could not add put operation to batch");

		saveIfBatchFull();
	}

	void RocksDatabase::del(size_t columnId, const rocksdb::Slice& key) {
		if (!m_pDb)
			CATAPULT_THROW_INVALID_ARGUMENT("RocksDatabase has not been initialized");

		// note: using SingleDelete can result in undefined result if value has ever been overwritten
		// that can't be guaranteed, so Delete is used instead
		auto status = m_pWriteBatch->Delete(m_handles[columnId], key);
		if (!status.ok())
			CATAPULT_THROW_DB_KEY_ERROR("could not add delete operation to batch");

		saveIfBatchFull();
	}

	size_t RocksDatabase::prune(size_t columnId, uint64_t boundary) {
		if (!m_pruningFilter.compactionFilter())
			return 0;

		m_pruningFilter.setPruningBoundary(boundary);
		m_pDb->CompactRange({}, m_handles[columnId], nullptr, nullptr);
		return m_pruningFilter.numRemoved();
	}

	void RocksDatabase::flush() {
		if (0 == m_pWriteBatch->GetDataSize())
			return;

		rocksdb::WriteOptions writeOptions;
		writeOptions.sync = true;

		auto directory = m_settings.DatabaseDirectory + "/";
		utils::SlowOperationLogger logger(utils::ExtractDirectoryName(directory.c_str()).pData, utils::LogLevel::warning);
		auto status = m_pDb->Write(writeOptions, m_pWriteBatch.get());
		if (!status.ok())
			CATAPULT_THROW_RUNTIME_ERROR_1("could not store batch in db", status.ToString());

		m_pWriteBatch->Clear();
	}

	void RocksDatabase::saveIfBatchFull() {
		if (m_pWriteBatch->GetDataSize() < m_settings.DatabaseConfig.MaxWriteBatchSize.bytes())
			return;

		flush();
	}

	// endregion
}}
