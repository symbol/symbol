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

#include "FileProofStorage.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/BufferedFileStream.h"
#include "catapult/io/PodIoUtils.h"

namespace catapult { namespace io {

	// region FinalizationIndexFile

	FileProofStorage::FinalizationIndexFile::FinalizationIndexFile(const std::string& filename, LockMode lockMode)
			: m_filename(filename)
			, m_lockMode(lockMode)
	{}

	bool FileProofStorage::FinalizationIndexFile::exists() const {
		return boost::filesystem::is_regular_file(m_filename);
	}

	model::FinalizationStatistics FileProofStorage::FinalizationIndexFile::get() const {
		auto indexFile = open(OpenMode::Read_Only);
		model::FinalizationStatistics finalizationStatistics;
		if (sizeof(model::FinalizationStatistics) != indexFile.size())
			return model::FinalizationStatistics();

		indexFile.read({ reinterpret_cast<uint8_t*>(&finalizationStatistics), sizeof(model::FinalizationStatistics) });
		return finalizationStatistics;
	}

	void FileProofStorage::FinalizationIndexFile::set(const model::FinalizationStatistics& finalizationStatistics) {
		auto indexFile = open(OpenMode::Read_Append);
		indexFile.seek(0);
		indexFile.write({ reinterpret_cast<const uint8_t*>(&finalizationStatistics), sizeof(model::FinalizationStatistics) });
	}

	RawFile FileProofStorage::FinalizationIndexFile::open(OpenMode mode) const {
		return RawFile(m_filename, mode, m_lockMode);
	}

	// endregion

	// region FileProofStorage

	FileProofStorage::FileProofStorage(const std::string& dataDirectory)
			: m_dataDirectory(dataDirectory)
			, m_epochHeightMapping(m_dataDirectory, "proof.heights")
			, m_indexFile((boost::filesystem::path(m_dataDirectory) / "proof.index.dat").generic_string())
	{}

	model::FinalizationStatistics FileProofStorage::statistics() const {
		if (!m_indexFile.exists())
			return model::FinalizationStatistics();

		return m_indexFile.get();
	}

	namespace {
		static constexpr auto Proof_File_Extension = ".proof";

		auto OpenProofFile(const std::string& baseDirectory, FinalizationEpoch epoch, OpenMode mode = OpenMode::Read_Only) {
			auto storageDir = config::CatapultStorageDirectoryPreparer::Prepare(baseDirectory, epoch);
			return std::make_unique<RawFile>(storageDir.storageFile(Proof_File_Extension), mode);
		}
	}

	namespace {
		std::shared_ptr<model::FinalizationProof> ReadFinalizationProof(RawFile& proofFile) {
			auto size = Read32(proofFile);
			proofFile.seek(0);

			auto pProof = utils::MakeSharedWithSize<model::FinalizationProof>(size);
			pProof->Size = size;
			proofFile.read({ reinterpret_cast<uint8_t*>(pProof.get()), size });
			return pProof;
		}
	}

	std::shared_ptr<const model::FinalizationProof> FileProofStorage::loadProof(FinalizationEpoch epoch) const {
		if (FinalizationEpoch() == epoch)
			CATAPULT_THROW_INVALID_ARGUMENT("loadProof called with epoch 0");

		auto currentEpoch = statistics().Round.Epoch;
		if (currentEpoch < epoch) {
			std::ostringstream out;
			out << "cannot load proof with epoch " << epoch << " when storage epoch is " << currentEpoch;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		auto pProofFile = OpenProofFile(m_dataDirectory, epoch);
		return ReadFinalizationProof(*pProofFile);
	}

	std::shared_ptr<const model::FinalizationProof> FileProofStorage::loadProof(Height height) const {
		if (Height() == height)
			CATAPULT_THROW_INVALID_ARGUMENT("loadProof called with height 0");

		auto currentHeight = statistics().Height;
		if (currentHeight < height) {
			std::ostringstream out;
			out << "cannot load proof with height " << height << " when storage height is " << currentHeight;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		auto epoch = findEpochForHeight(height);
		if (FinalizationEpoch() == epoch)
			return nullptr;

		auto pProofFile = OpenProofFile(m_dataDirectory, epoch);
		return ReadFinalizationProof(*pProofFile);
	}

	void FileProofStorage::saveProof(const model::FinalizationProof& proof) {
		auto currentStatistics = statistics();
		if (currentStatistics.Round > proof.Round || proof.Round.Epoch > currentStatistics.Round.Epoch + FinalizationEpoch(1)) {
			std::ostringstream out;
			out << "cannot save proof with round " << proof.Round << " when storage round is " << currentStatistics.Round;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		if (currentStatistics.Height > proof.Height) {
			std::ostringstream out;
			out << "cannot save proof with height " << proof.Height << " when storage height is " << currentStatistics.Height;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		{
			auto pProofFile = OpenProofFile(m_dataDirectory, proof.Round.Epoch, OpenMode::Read_Write);
			BufferedOutputFileStream stream(std::move(*pProofFile));
			stream.write({ reinterpret_cast<const uint8_t*>(&proof), proof.Size });
			stream.flush();
		}

		m_epochHeightMapping.save(proof.Round.Epoch, proof.Height);

		m_indexFile.set({ proof.Round, proof.Height, proof.Hash });
	}

	FinalizationEpoch FileProofStorage::findEpochForHeight(Height height) const {
		constexpr auto Max_Epochs_In_Batch = FinalizationEpoch(100);
		auto currentEpoch = statistics().Round.Epoch;

		while (true) {
			auto rangeBeginEpoch = currentEpoch > Max_Epochs_In_Batch ? (currentEpoch - Max_Epochs_In_Batch) : FinalizationEpoch(1);
			auto numHeights = currentEpoch > Max_Epochs_In_Batch ? Max_Epochs_In_Batch.unwrap() : currentEpoch.unwrap();
			auto heights = m_epochHeightMapping.loadRangeFrom(rangeBeginEpoch, numHeights);

			// heights are in ascending order, so if height is not present in this 'batch', load next one
			if (!heights.empty() && *heights.cbegin() > height) {
				currentEpoch = rangeBeginEpoch;
				continue;
			}

			auto iter = std::upper_bound(heights.cbegin(), heights.cend(), height);
			if (height == *--iter)
				return rangeBeginEpoch + FinalizationEpoch(static_cast<uint64_t>(std::distance(heights.cbegin(), iter)));

			CATAPULT_LOG(debug) << "element not found, was looking for " << height;
			return FinalizationEpoch();
		}
	}

	// endregion
}}
