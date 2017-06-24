#include "LocalNodeStateStorage.h"
#include "catapult/cache/CacheStorageAdapter.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/SupplementalData.h"
#include "catapult/cache/SupplementalDataStorage.h"
#include "catapult/io/BufferedFileStream.h"
#include "catapult/utils/StackLogger.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>

namespace catapult { namespace local { namespace p2p {

	namespace {
		constexpr size_t Default_Loader_Batch_Size = 100'000;
		constexpr auto Supplemental_Data_Filename = "supplemental.dat";

		std::string GetStatePath(const std::string& baseDirectory, const std::string& filename) {
			boost::filesystem::path path = baseDirectory;
			path /= "state";
			if (!boost::filesystem::exists(path))
				boost::filesystem::create_directory(path);
			path /= filename;
			return path.generic_string();
		}

		void LoadCache(
				const std::string& baseDirectory,
				const std::string& filename,
				cache::CacheStorage& cacheStorage) {
			auto path = GetStatePath(baseDirectory, filename);
			io::BufferedInputFileStream file(io::RawFile(path.c_str(), io::OpenMode::Read_Only));
			cacheStorage.loadAll(file, Default_Loader_Batch_Size);
		}

		void SaveCache(
				const std::string& baseDirectory,
				const std::string& filename,
				const cache::CacheStorage& cacheStorage) {
			auto path = GetStatePath(baseDirectory, filename);
			io::BufferedOutputFileStream file(io::RawFile(path.c_str(), io::OpenMode::Read_Write));
			cacheStorage.saveAll(file);
		}

		bool HasSupplementalData(const std::string& baseDirectory) {
			auto path = GetStatePath(baseDirectory, Supplemental_Data_Filename);
			return boost::filesystem::exists(path);
		}

		std::string GetStorageFilename(const cache::CacheStorage& storage) {
			return storage.name() + ".dat";
		}
	}

	bool LoadState(const std::string& dataDirectory, cache::CatapultCache& cache, cache::SupplementalData& supplementalData) {
		if (!HasSupplementalData(dataDirectory))
			return false;

		utils::StackLogger stopwatch("load state", utils::LogLevel::Warning);

		for (const auto& pStorage : cache.storages())
			LoadCache(dataDirectory, GetStorageFilename(*pStorage), *pStorage);

		Height chainHeight;
		{
			auto path = GetStatePath(dataDirectory, Supplemental_Data_Filename);
			io::BufferedInputFileStream file(io::RawFile(path.c_str(), io::OpenMode::Read_Only));
			cache::LoadSupplementalData(file, supplementalData, chainHeight);
		}

		auto cacheDelta = cache.createDelta();
		cache.commit(chainHeight);
		return true;
	}

	void SaveState(const std::string& dataDirectory, const cache::CatapultCache& cache, const cache::SupplementalData& supplementalData) {
		for (const auto& pStorage : cache.storages())
			SaveCache(dataDirectory, GetStorageFilename(*pStorage), *pStorage);

		{
			auto path = GetStatePath(dataDirectory, Supplemental_Data_Filename);
			io::BufferedOutputFileStream file(io::RawFile(path.c_str(), io::OpenMode::Read_Write));

			cache::SupplementalData data;
			data.State = supplementalData.State;
			data.ChainScore = supplementalData.ChainScore;
			cache::SaveSupplementalData(data, cache.createView().height(), file);
		}
	}
}}}
