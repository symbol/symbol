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

#include "StorageImportanceCalculatorFactory.h"
#include "catapult/cache/StateVersion.h"
#include "catapult/cache_core/AccountStateCacheDelta.h"
#include "catapult/io/FileStream.h"
#include "catapult/io/IndexFile.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/state/AccountStateSerializer.h"

namespace catapult { namespace importance {

	namespace {
		constexpr auto Index_Filename = "index.dat";
		constexpr auto Empty_Bucket_Sentinel = std::numeric_limits<uint32_t>::max();

		// region utils

		std::string GetFilename(model::ImportanceHeight importanceHeight) {
			std::ostringstream out;
			out << utils::HexFormat(importanceHeight) << ".dat";
			return out.str();
		}

		// endregion

		// region serialization

#pragma pack(push, 1)

		struct PackedAccountEntry {
		public:
			static constexpr uint16_t State_Version = 1;

		public:
			// corresponds to current importance height
			catapult::Address Address;
			Amount HarvestingBalance; // informational
			catapult::Importance Importance;

			// corresponds to previous importance height
			Amount TotalFeesPaid;
			uint32_t BeneficiaryCount;
			uint64_t RawScore;
		};

#pragma pack(pop)

		// endregion

		// region WriteDecorator

		class WriteDecorator final : public ImportanceCalculator {
		public:
			WriteDecorator(
					const model::BlockChainConfiguration& config,
					const config::CatapultDirectory& directory,
					std::unique_ptr<ImportanceCalculator>&& pCalculator)
					: m_config(config)
					, m_directory(directory)
					, m_pCalculator(std::move(pCalculator))
			{}

		public:
			void recalculate(
					ImportanceRollbackMode mode,
					model::ImportanceHeight importanceHeight,
					cache::AccountStateCacheDelta& cache) const override {
				m_pCalculator->recalculate(mode, importanceHeight, cache);

				if (ImportanceRollbackMode::Disabled == mode)
					return;

				writeToFile(importanceHeight, cache);
			}

		private:
			void writeToFile(model::ImportanceHeight importanceHeight, const cache::AccountStateCacheDelta& cache) const {
				auto filename = m_directory.file(GetFilename(importanceHeight));
				CATAPULT_LOG(debug) << "writing importances to file " << filename << " for height " << importanceHeight;

				io::FileStream output(io::RawFile(filename, io::OpenMode::Read_Write));
				cache::StateVersion<PackedAccountEntry>::Write(output);

				const auto& highValueAccounts = cache.highValueAccounts();
				io::Write64(output, highValueAccounts.addresses().size());
				io::Write64(output, highValueAccounts.removedAddresses().size());

				writeAll(output, cache, highValueAccounts.addresses());
				writeAll(output, cache, highValueAccounts.removedAddresses());

				io::IndexFile(m_directory.file(Index_Filename)).set(importanceHeight.unwrap());
			}

			void writeAll(io::OutputStream& output, const cache::AccountStateCacheDelta& cache, const model::AddressSet& addresses) const {
				for (const auto& address : addresses) {
					auto accountStateIter = cache.find(address);

					auto entry = pack(accountStateIter.get());
					output.write({ reinterpret_cast<const uint8_t*>(&entry), sizeof(PackedAccountEntry) });
				}
			}

			PackedAccountEntry pack(const state::AccountState& accountState) const {
				PackedAccountEntry entry;
				entry.Address = accountState.Address;
				entry.HarvestingBalance = accountState.Balances.get(m_config.HarvestingMosaicId);

				// ImportanceHeight isn't needed because it is derivable from file
				entry.Importance = accountState.ImportanceSnapshots.current();

				auto previousBucketIter = ++accountState.ActivityBuckets.begin();
				entry.TotalFeesPaid = previousBucketIter->TotalFeesPaid;
				entry.BeneficiaryCount = previousBucketIter->BeneficiaryCount;
				entry.RawScore = previousBucketIter->RawScore;

				// each file contains an importance snapshot for the current height and a bucket for the previous height
				// set a sentinel value if the bucket (not snapshot) is empty to deserialize properly
				auto isNemesisSnapshot = model::ImportanceHeight(1) == accountState.ImportanceSnapshots.height();
				auto isCorrespondingBucketEmpty = model::ImportanceHeight() == previousBucketIter->StartHeight;
				if (!isNemesisSnapshot && isCorrespondingBucketEmpty)
					entry.BeneficiaryCount = Empty_Bucket_Sentinel;

				return entry;
			}

		private:
			const model::BlockChainConfiguration& m_config;
			config::CatapultDirectory m_directory;
			std::unique_ptr<ImportanceCalculator> m_pCalculator;
		};

		// endregion

		// region ReadDecorator

		class ReadDecorator final : public ImportanceCalculator {
		public:
			ReadDecorator(
					const model::BlockChainConfiguration& config,
					const config::CatapultDirectory& directory,
					std::unique_ptr<ImportanceCalculator>&& pCalculator)
					: m_config(config)
					, m_directory(directory)
					, m_pCalculator(std::move(pCalculator))
			{}

		public:
			void recalculate(
					ImportanceRollbackMode mode,
					model::ImportanceHeight importanceHeight,
					cache::AccountStateCacheDelta& cache) const override {
				if (ImportanceRollbackMode::Disabled == mode)
					CATAPULT_THROW_INVALID_ARGUMENT("cannot rollback importances when rollback is disabled");

				auto groupingFacade = model::HeightGroupingFacade<model::ImportanceHeight>(importanceHeight, m_config.ImportanceGrouping);

				// if the most recently calculated importance height is being rolled back, bypass file loading because it will be in memory
				auto lastImportanceHeight = model::ImportanceHeight(io::IndexFile(m_directory.file(Index_Filename)).get());
				if (groupingFacade.next(1) == lastImportanceHeight) {
					m_pCalculator->recalculate(mode, importanceHeight, cache);
					return;
				}

				CATAPULT_LOG(debug) << "restoring older importances from file for height " << importanceHeight;

				ReadManager readManager(
						groupingFacade.previous(Activity_Bucket_History_Size - Rollback_Buffer_Size - 2),
						groupingFacade.next(1),
						m_config.ImportanceGrouping);
				readFromFiles(readManager, cache);
			}

		private:
			class ReadManager {
			public:
				ReadManager(model::ImportanceHeight startImportanceHeight, model::ImportanceHeight endImportanceHeight, uint64_t grouping)
						: m_endImportanceHeight(endImportanceHeight)
						, m_groupingFacade(startImportanceHeight, grouping)
				{}

			public:
				bool shouldProcess(size_t id) const {
					return m_groupingFacade.next(id) <= m_endImportanceHeight;
				}

				bool shouldSetImportance(size_t id) const {
					return m_groupingFacade.next(id + 1) == m_endImportanceHeight;
				}

				bool shouldSetActivityBucket(size_t id) const {
					return 0 != id || model::ImportanceHeight(1) != current(id);
				}

				model::ImportanceHeight current(size_t id) const {
					return m_groupingFacade.next(id);
				}

				model::ImportanceHeight bucketHeight(size_t id) const {
					return 0 == id ? m_groupingFacade.previous(1) : m_groupingFacade.next(id - 1);
				}

			private:
				model::ImportanceHeight m_endImportanceHeight;
				model::HeightGroupingFacade<model::ImportanceHeight> m_groupingFacade;
			};

			void readFromFiles(const ReadManager& readManager, cache::AccountStateCacheDelta& cache) const {
				model::AddressSet addresses;
				for (auto i = 0u; readManager.shouldProcess(i); ++i) {
					processFile(readManager.current(i), [&readManager, &addresses, &cache, i](const auto& entry) {
						auto accountStateIter = cache.find(entry.Address);
						if (addresses.cend() == addresses.find(entry.Address)) {
							Clear(accountStateIter.get());
							addresses.insert(entry.Address);
						}

						if (readManager.shouldSetImportance(i)) {
							if (Importance() != entry.Importance)
								accountStateIter.get().ImportanceSnapshots.set(entry.Importance, readManager.current(i));
							else
								accountStateIter.get().ImportanceSnapshots.push();
						}

						if (readManager.shouldSetActivityBucket(i)) {
							if (Empty_Bucket_Sentinel != entry.BeneficiaryCount) {
								accountStateIter.get().ActivityBuckets.update(readManager.bucketHeight(i), [&entry](auto& bucket) {
									bucket.TotalFeesPaid = entry.TotalFeesPaid;
									bucket.BeneficiaryCount = entry.BeneficiaryCount;
									bucket.RawScore = entry.RawScore;
								});
							} else {
								accountStateIter.get().ActivityBuckets.push();
							}
						}
					});
				}
			}

			void processFile(model::ImportanceHeight importanceHeight, const consumer<const PackedAccountEntry&>& process) const {
				auto filename = m_directory.file(GetFilename(importanceHeight));
				CATAPULT_LOG(debug) << "restoring older importances from file " << filename << " for height " << importanceHeight;

				io::FileStream input(io::RawFile(filename, io::OpenMode::Read_Only));
				cache::StateVersion<PackedAccountEntry>::ReadAndCheck(input);

				auto count = io::Read64(input);
				auto removedCount = io::Read64(input);

				for (auto i = 0u; i < count + removedCount; ++i) {
					PackedAccountEntry entry;
					input.read({ reinterpret_cast<uint8_t*>(&entry), sizeof(PackedAccountEntry) });
					process(entry);
				}
			}

		private:
			static void Clear(state::AccountState& accountState) {
				accountState.ImportanceSnapshots = {};
				accountState.ActivityBuckets = {};
			}

		private:
			const model::BlockChainConfiguration& m_config;
			config::CatapultDirectory m_directory;
			std::unique_ptr<ImportanceCalculator> m_pCalculator;
		};

		// endregion
	}

	// region StorageImportanceCalculatorFactory

	StorageImportanceCalculatorFactory::StorageImportanceCalculatorFactory(const model::BlockChainConfiguration& config) : m_config(config)
	{}

	std::unique_ptr<ImportanceCalculator> StorageImportanceCalculatorFactory::createWriteCalculator(
			std::unique_ptr<ImportanceCalculator>&& pCalculator,
			const config::CatapultDirectory& directory) const {
		return std::make_unique<WriteDecorator>(m_config, directory, std::move(pCalculator));
	}

	std::unique_ptr<ImportanceCalculator> StorageImportanceCalculatorFactory::createReadCalculator(
			std::unique_ptr<ImportanceCalculator>&& pCalculator,
			const config::CatapultDirectory& directory) const {
		return std::make_unique<ReadDecorator>(m_config, directory, std::move(pCalculator));
	}

	// endregion
}}
