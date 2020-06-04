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

#include "LocalNodeStateFileStorage.h"
#include "LocalNodeChainScore.h"
#include "LocalNodeStateRef.h"
#include "NemesisBlockLoader.h"
#include "catapult/cache/CacheStorage.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/SupplementalDataStorage.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/config/NodeConfiguration.h"
#include "catapult/consumers/BlockChainSyncHandlers.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/io/BufferedFileStream.h"
#include "catapult/io/FilesystemUtils.h"
#include "catapult/io/IndexFile.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace extensions {

	// region constants + utils

	namespace {
		constexpr size_t Default_Loader_Batch_Size = 100'000;
		constexpr auto Supplemental_Data_Filename = "supplemental.dat";

		std::string GetStorageFilename(const cache::CacheStorage& storage) {
			return storage.name() + ".dat";
		}
	}

	// endregion

	// region HasSerializedState

	bool HasSerializedState(const config::CatapultDirectory& directory) {
		return boost::filesystem::exists(directory.file(Supplemental_Data_Filename));
	}

	// endregion

	// region LoadDependentStateFromDirectory

	namespace {
		io::BufferedInputFileStream OpenInputStream(const config::CatapultDirectory& directory, const std::string& filename) {
			return io::BufferedInputFileStream(io::RawFile(directory.file(filename), io::OpenMode::Read_Only));
		}

		void LoadDependentStateFromDirectory(
				const config::CatapultDirectory& directory,
				cache::CatapultCache& cache,
				cache::SupplementalData& supplementalData) {
			// load supplemental data
			Height chainHeight;
			{
				auto inputStream = OpenInputStream(directory, Supplemental_Data_Filename);
				cache::LoadSupplementalData(inputStream, supplementalData, chainHeight);
			}

			// commit changes
			auto cacheDelta = cache.createDelta();
			cacheDelta.dependentState() = supplementalData.State;
			cache.commit(chainHeight);
		}
	}

	void LoadDependentStateFromDirectory(const config::CatapultDirectory& directory, cache::CatapultCache& cache) {
		cache::SupplementalData supplementalData;
		LoadDependentStateFromDirectory(directory, cache, supplementalData);
	}

	// endregion

	// region LoadStateFromDirectory

	namespace {
		bool LoadStateFromDirectory(
				const config::CatapultDirectory& directory,
				cache::CatapultCache& cache,
				cache::SupplementalData& supplementalData) {
			if (!HasSerializedState(directory))
				return false;

			// 1. load cache data
			utils::StackLogger stopwatch("load state", utils::LogLevel::important);
			for (const auto& pStorage : cache.storages()) {
				auto inputStream = OpenInputStream(directory, GetStorageFilename(*pStorage));
				pStorage->loadAll(inputStream, Default_Loader_Batch_Size);
			}

			// 2. load supplemental data
			LoadDependentStateFromDirectory(directory, cache, supplementalData);
			return true;
		}
	}

	StateHeights LoadStateFromDirectory(
			const config::CatapultDirectory& directory,
			const LocalNodeStateRef& stateRef,
			const plugins::PluginManager& pluginManager) {
		cache::SupplementalData supplementalData;
		if (LoadStateFromDirectory(directory, stateRef.Cache, supplementalData)) {
			stateRef.Score += supplementalData.ChainScore;
		} else {
			auto cacheDelta = stateRef.Cache.createDelta();
			NemesisBlockLoader loader(cacheDelta, pluginManager, pluginManager.createObserver());
			loader.executeAndCommit(stateRef, StateHashVerification::Enabled);
			stateRef.Score += model::ChainScore(1); // set chain score to 1 after processing nemesis
		}

		StateHeights heights;
		heights.Cache = stateRef.Cache.createView().height();
		heights.Storage = stateRef.Storage.view().chainHeight();
		return heights;
	}

	// endregion

	// region LocalNodeStateSerializer

	namespace {
		io::BufferedOutputFileStream OpenOutputStream(const config::CatapultDirectory& directory, const std::string& filename) {
			return io::BufferedOutputFileStream(io::RawFile(directory.file(filename), io::OpenMode::Read_Write));
		}

		void SaveStateToDirectory(
				const config::CatapultDirectory& directory,
				const std::vector<std::unique_ptr<const cache::CacheStorage>>& cacheStorages,
				const state::CatapultState& state,
				const model::ChainScore& score,
				Height height,
				const consumer<const cache::CacheStorage&, io::OutputStream&>& save) {
			// 1. create directory if required
			if (!boost::filesystem::exists(directory.path()))
				boost::filesystem::create_directory(directory.path());

			// 2. save cache data
			for (const auto& pStorage : cacheStorages) {
				auto outputStream = OpenOutputStream(directory, GetStorageFilename(*pStorage));
				save(*pStorage, outputStream);
			}

			// 3. save supplemental data
			cache::SupplementalData supplementalData{ state, score };
			auto outputStream = OpenOutputStream(directory, Supplemental_Data_Filename);
			cache::SaveSupplementalData(supplementalData, height, outputStream);
		}
	}

	LocalNodeStateSerializer::LocalNodeStateSerializer(const config::CatapultDirectory& directory) : m_directory(directory)
	{}

	void LocalNodeStateSerializer::save(const cache::CatapultCache& cache, const model::ChainScore& score) const {
		auto cacheStorages = cache.storages();
		auto cacheView = cache.createView();
		const auto& state = cacheView.dependentState();
		auto height = cacheView.height();
		SaveStateToDirectory(m_directory, cacheStorages, state, score, height, [&cacheView](const auto& storage, auto& outputStream) {
			storage.saveAll(cacheView, outputStream);
		});
	}

	void LocalNodeStateSerializer::save(
			const cache::CatapultCacheDelta& cacheDelta,
			const std::vector<std::unique_ptr<const cache::CacheStorage>>& cacheStorages,
			const model::ChainScore& score,
			Height height) const {
		const auto& state = cacheDelta.dependentState();
		SaveStateToDirectory(m_directory, cacheStorages, state, score, height, [&cacheDelta](const auto& storage, auto& outputStream) {
			storage.saveSummary(cacheDelta, outputStream);
		});
	}

	void LocalNodeStateSerializer::moveTo(const config::CatapultDirectory& destinationDirectory) {
		io::PurgeDirectory(destinationDirectory.str());
		boost::filesystem::remove(destinationDirectory.path());
		boost::filesystem::rename(m_directory.path(), destinationDirectory.path());
	}

	// endregion

	// region SaveStateToDirectoryWithCheckpointing

	namespace {
		void SetCommitStep(const config::CatapultDataDirectory& dataDirectory, consumers::CommitOperationStep step) {
			io::IndexFile(dataDirectory.rootDir().file("commit_step.dat")).set(utils::to_underlying_type(step));
		}
	}

	void SaveStateToDirectoryWithCheckpointing(
			const config::CatapultDataDirectory& dataDirectory,
			const config::NodeConfiguration& nodeConfig,
			const cache::CatapultCache& cache,
			const model::ChainScore& score) {
		SetCommitStep(dataDirectory, consumers::CommitOperationStep::Blocks_Written);

		LocalNodeStateSerializer serializer(dataDirectory.dir("state.tmp"));

		if (nodeConfig.EnableCacheDatabaseStorage) {
			auto storages = const_cast<const cache::CatapultCache&>(cache).storages();
			auto height = cache.createView().height();

			auto cacheDetachableDelta = cache.createDetachableDelta();
			auto cacheDetachedDelta = cacheDetachableDelta.detach();
			auto pCacheDelta = cacheDetachedDelta.tryLock();

			serializer.save(*pCacheDelta, storages, score, height);
		} else {
			serializer.save(cache, score);
		}

		SetCommitStep(dataDirectory, consumers::CommitOperationStep::State_Written);

		serializer.moveTo(dataDirectory.dir("state"));

		SetCommitStep(dataDirectory, consumers::CommitOperationStep::All_Updated);
	}

	// endregion
}}
