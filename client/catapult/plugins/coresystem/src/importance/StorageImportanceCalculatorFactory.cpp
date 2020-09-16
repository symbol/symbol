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

#include "StorageImportanceCalculatorFactory.h"
#include "catapult/cache/StateVersion.h"
#include "catapult/cache_core/AccountStateCacheDelta.h"
#include "catapult/io/FileStream.h"
#include "catapult/io/IndexFile.h"
#include "catapult/state/AccountStateSerializer.h"

namespace catapult { namespace importance {

	namespace {
		using Serializer = state::AccountStateNonHistoricalSerializer;

		constexpr auto Index_Filename = "index.dat";

		// region utils

		std::string GetFilename(model::ImportanceHeight importanceHeight) {
			std::ostringstream out;
			out << utils::HexFormat(importanceHeight) << ".dat";
			return out.str();
		}

		// endregion

		// region WriteDecorator

		class WriteDecorator final : public ImportanceCalculator {
		public:
			WriteDecorator(std::unique_ptr<ImportanceCalculator>&& pCalculator, const config::CatapultDirectory& directory)
					: m_pCalculator(std::move(pCalculator))
					, m_directory(directory)
			{}

		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				m_pCalculator->recalculate(importanceHeight, cache);
				writeToFile(importanceHeight, cache);
			}

		private:
			void writeToFile(model::ImportanceHeight importanceHeight, const cache::AccountStateCacheDelta& cache) const {
				io::FileStream output(io::RawFile(m_directory.file(GetFilename(importanceHeight)), io::OpenMode::Read_Write));
				cache::StateVersion<Serializer>::Write(output);

				const auto& highValueAddresses = cache.highValueAccounts().addresses();
				io::Write64(output, highValueAddresses.size());

				for (const auto& address : highValueAddresses) {
					auto accountStateIter = cache.find(address);
					Serializer::Save(accountStateIter.get(), output);
				}

				io::IndexFile(m_directory.file(Index_Filename)).set(importanceHeight.unwrap());
			}

		private:
			std::unique_ptr<ImportanceCalculator> m_pCalculator;
			config::CatapultDirectory m_directory;
		};

		// endregion

		// region ReadDecorator

		class ReadDecorator final : public ImportanceCalculator {
		public:
			ReadDecorator(std::unique_ptr<ImportanceCalculator>&& pCalculator, const config::CatapultDirectory& directory)
					: m_pCalculator(std::move(pCalculator))
					, m_directory(directory)
			{}

		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				m_pCalculator->recalculate(importanceHeight, cache);

				// if the most recently calculated importance height is being rolled back, bypass file loading because it will be in memory
				auto lastImportanceHeight = model::ImportanceHeight(io::IndexFile(m_directory.file(Index_Filename)).get());
				if (lastImportanceHeight == importanceHeight)
					return;

				readFromFile(importanceHeight, cache);
			}

		private:
			void readFromFile(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const {
				auto filename = m_directory.file(GetFilename(importanceHeight));
				CATAPULT_LOG(debug) << "restoring older importances from file " << filename << " for height " << importanceHeight;

				io::FileStream input(io::RawFile(filename, io::OpenMode::Read_Only));
				cache::StateVersion<Serializer>::ReadAndCheck(input);

				auto numAccounts = io::Read64(input);
				for (auto i = 0u; i < numAccounts; ++i) {
					auto loadedAccountState = Serializer::Load(input);
					auto cacheAccountStateIter = cache.find(loadedAccountState.Address);

					cacheAccountStateIter.get().ImportanceSnapshots = loadedAccountState.ImportanceSnapshots;
					cacheAccountStateIter.get().ActivityBuckets = loadedAccountState.ActivityBuckets;
				}
			}

		private:
			std::unique_ptr<ImportanceCalculator> m_pCalculator;
			config::CatapultDirectory m_directory;
		};

		// endregion
	}

	// region StorageImportanceCalculatorFactory

	StorageImportanceCalculatorFactory::StorageImportanceCalculatorFactory(const config::CatapultDirectory& directory)
			: m_directory(directory)
	{}

	std::unique_ptr<ImportanceCalculator> StorageImportanceCalculatorFactory::createWriteCalculator(
			std::unique_ptr<ImportanceCalculator>&& pCalculator) const {
		return std::make_unique<WriteDecorator>(std::move(pCalculator), m_directory);
	}

	std::unique_ptr<ImportanceCalculator> StorageImportanceCalculatorFactory::createReadCalculator(
			std::unique_ptr<ImportanceCalculator>&& pCalculator) const {
		return std::make_unique<ReadDecorator>(std::move(pCalculator), m_directory);
	}

	// endregion
}}
