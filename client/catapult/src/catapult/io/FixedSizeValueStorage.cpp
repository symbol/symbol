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

#include "FixedSizeValueStorage.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <inttypes.h>

namespace catapult { namespace io {

	namespace {
		static constexpr uint64_t Unset_Directory_Id = std::numeric_limits<uint64_t>::max();
		static constexpr uint32_t Files_Per_Directory = 65536u;

#ifdef _MSC_VER
#define SPRINTF sprintf_s
#else
#define SPRINTF sprintf
#endif

		boost::filesystem::path GetDirectoryPath(const std::string& baseDirectory, uint64_t identifier) {
			char subDirectory[16];
			(SPRINTF(subDirectory, "%05" PRId64, identifier / Files_Per_Directory));
			boost::filesystem::path path = baseDirectory;
			path /= subDirectory;
			if (!boost::filesystem::exists(path))
				boost::filesystem::create_directory(path);

			return path;
		}

		boost::filesystem::path GetFixedSizeValueStoragePath(
				const std::string& baseDirectory,
				const std::string& prefix,
				uint64_t identifier) {
			auto path = GetDirectoryPath(baseDirectory, identifier);
			path /= prefix + ".dat";
			return path;
		}
	}

	template<typename TKey, typename TValue>
	FixedSizeValueStorage<TKey, TValue>::FixedSizeValueStorage(const std::string& dataDirectory, const std::string& prefix)
			: m_dataDirectory(dataDirectory)
			, m_prefix(prefix)
			, m_cachedDirectoryId(Unset_Directory_Id)
	{}

	template<typename TKey, typename TValue>
	model::EntityRange<TValue> FixedSizeValueStorage<TKey, TValue>::loadRangeFrom(TKey key, size_t numValues) const {
		uint8_t* pData = nullptr;
		auto range = model::EntityRange<TValue>::PrepareFixed(numValues, &pData);
		while (numValues) {
			auto pFixedSizeValueStorage = openStorageFile(key, OpenMode::Read_Only);
			seekStorageFile(*pFixedSizeValueStorage, key);

			auto count = Files_Per_Directory - (key.unwrap() % Files_Per_Directory);
			count = std::min<size_t>(numValues, count);

			pFixedSizeValueStorage->read(MutableRawBuffer(pData, count * TValue::Size));

			pData += count * TValue::Size;
			numValues -= count;
			key = key + TKey(count);
		}

		return range;
	}

	template<typename TKey, typename TValue>
	void FixedSizeValueStorage<TKey, TValue>::save(TKey key, const TValue& value) {
		auto currentId = key.unwrap() / Files_Per_Directory;
		if (m_cachedDirectoryId != currentId) {
			m_pCachedStorageFile = openStorageFile(key, OpenMode::Read_Append);
			m_cachedDirectoryId = currentId;
		}

		seekStorageFile(*m_pCachedStorageFile, key);
		m_pCachedStorageFile->write({ reinterpret_cast<const uint8_t*>(&value), TValue::Size });
	}

	template<typename TKey, typename TValue>
	void FixedSizeValueStorage<TKey, TValue>::reset() {
		m_cachedDirectoryId = Unset_Directory_Id;
		m_pCachedStorageFile.reset();
	}

	template<typename TKey, typename TValue>
	std::unique_ptr<RawFile> FixedSizeValueStorage<TKey, TValue>::openStorageFile(TKey key, OpenMode openMode) const {
		auto filePath = GetFixedSizeValueStoragePath(m_dataDirectory, m_prefix, key.unwrap());
		auto pStorageFile = std::make_unique<RawFile>(filePath.generic_string(), openMode, LockMode::None);

		// check that first storage file has at least two values inside
		if (key.unwrap() < Files_Per_Directory && TValue::Size * 2 > pStorageFile->size())
			CATAPULT_THROW_RUNTIME_ERROR_2("storage file has invalid size", filePath.generic_string(), pStorageFile->size());

		return pStorageFile;
	}

	template<typename TKey, typename TValue>
	void FixedSizeValueStorage<TKey, TValue>::seekStorageFile(RawFile& hashFile, TKey key) const {
		auto index = key.unwrap() % Files_Per_Directory;
		hashFile.seek(index * TValue::Size);
	}

	// instantiation
	template class FixedSizeValueStorage<Height, Hash256>;
	template class FixedSizeValueStorage<FinalizationPoint, HeightHashPair>;
}}
