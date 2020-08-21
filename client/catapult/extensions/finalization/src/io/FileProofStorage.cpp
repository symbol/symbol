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
			, m_pointHeightMapping(m_dataDirectory, "proof.heights")
			, m_indexFile((boost::filesystem::path(m_dataDirectory) / "proof.index.dat").generic_string())
	{}

	model::FinalizationStatistics FileProofStorage::statistics() const {
		if (!m_indexFile.exists())
			return model::FinalizationStatistics();

		return m_indexFile.get();
	}

	namespace {
		static constexpr auto Proof_File_Extension = ".proof";

		auto OpenProofFile(const std::string& baseDirectory, FinalizationPoint point, OpenMode mode = OpenMode::Read_Only) {
			auto storageDir = config::CatapultStorageDirectoryPreparer::Prepare(baseDirectory, point);
			return std::make_unique<RawFile>(storageDir.storageFile(Proof_File_Extension), mode);
		}

		auto PrepareHeader(size_t proofSize, const model::FinalizationMessage& message, Height finalizedHeight) {
			model::PackedFinalizationProof header;
			header.VoteProofsCount = utils::checked_cast<size_t, uint32_t>(proofSize);
			header.Size = utils::checked_cast<size_t, uint32_t>(model::PackedFinalizationProof::CalculateRealSize(header));
			header.FinalizedHash = *message.HashesPtr();
			header.FinalizedHeight = finalizedHeight;
			header.StepIdentifier = message.StepIdentifier;
			return header;
		}
	}

	namespace {
		std::shared_ptr<model::PackedFinalizationProof> ReadPackedFinalizationProof(RawFile& proofFile) {
			auto size = Read32(proofFile);
			proofFile.seek(0);

			auto pProof = utils::MakeSharedWithSize<model::PackedFinalizationProof>(size);
			pProof->Size = size;
			proofFile.read({ reinterpret_cast<uint8_t*>(pProof.get()), size });
			return pProof;
		}
	}

	std::shared_ptr<const model::PackedFinalizationProof> FileProofStorage::loadProof(FinalizationPoint point) const {
		if (FinalizationPoint() == point)
			CATAPULT_THROW_INVALID_ARGUMENT("loadProof called with point 0");

		auto currentPoint = statistics().Point;
		if (currentPoint < point) {
			std::ostringstream out;
			out << "cannot load proof with point " << point << " when storage point is " << currentPoint;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		auto pProofFile = OpenProofFile(m_dataDirectory, point);
		return ReadPackedFinalizationProof(*pProofFile);
	}

	std::shared_ptr<const model::PackedFinalizationProof> FileProofStorage::loadProof(Height height) const {
		if (Height() == height)
			CATAPULT_THROW_INVALID_ARGUMENT("loadProof called with height 0");

		auto currentHeight = statistics().Height;
		if (currentHeight < height) {
			std::ostringstream out;
			out << "cannot load proof with height " << height << " when storage height is " << currentHeight;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		auto point = findPointForHeight(height);
		if (FinalizationPoint() == point)
			return nullptr;

		auto pProofFile = OpenProofFile(m_dataDirectory, point);
		return ReadPackedFinalizationProof(*pProofFile);
	}

	void FileProofStorage::saveProof(Height height, const FinalizationProof& proof) {
		if (proof.empty())
			CATAPULT_THROW_INVALID_ARGUMENT("cannot save empty proof");

		auto currentStatistics = statistics();
		const auto& firstMessage = *proof.front();
		auto messagePoint = FinalizationPoint(firstMessage.StepIdentifier.Point);
		if (messagePoint != currentStatistics.Point + FinalizationPoint(1)) {
			std::ostringstream out;
			out << "cannot save proof with point " << messagePoint << " when storage point is " << currentStatistics.Point;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		if (currentStatistics.Height > height) {
			std::ostringstream out;
			out << "cannot save proof with height " << height << " when storage height is " << currentStatistics.Height;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		{
			auto pProofFile = OpenProofFile(m_dataDirectory, messagePoint, OpenMode::Read_Write);
			BufferedOutputFileStream stream(std::move(*pProofFile));

			// write header
			auto header = PrepareHeader(proof.size(), firstMessage, height);
			stream.write({ reinterpret_cast<const uint8_t*>(&header), sizeof(model::PackedFinalizationProof) });

			// write vote proofs
			for (const auto& pMessage : proof)
				stream.write({ reinterpret_cast<const uint8_t*>(&pMessage->Signature), sizeof(crypto::OtsTreeSignature) });

			stream.flush();
		}

		m_pointHeightMapping.save(messagePoint, height);
		m_indexFile.set({ messagePoint, height, *firstMessage.HashesPtr() });
	}

	FinalizationPoint FileProofStorage::findPointForHeight(Height height) const {
		constexpr auto Max_Points_In_Batch = FinalizationPoint(100);
		auto currentPoint = statistics().Point;

		while (true) {
			auto rangeBeginPoint = currentPoint > Max_Points_In_Batch ? (currentPoint - Max_Points_In_Batch) : FinalizationPoint(1);
			auto numHeights = currentPoint > Max_Points_In_Batch ? Max_Points_In_Batch.unwrap() : currentPoint.unwrap();
			auto heights = m_pointHeightMapping.loadRangeFrom(rangeBeginPoint, numHeights);

			// heights are in ascending order, so if height is not present in this 'batch', load next one
			if (!heights.empty() && *heights.cbegin() > height) {
				currentPoint = rangeBeginPoint;
				continue;
			}

			auto iter = std::upper_bound(heights.cbegin(), heights.cend(), height);
			if (height == *--iter)
				return rangeBeginPoint + FinalizationPoint(static_cast<uint64_t>(std::distance(heights.cbegin(), iter)));

			CATAPULT_LOG(debug) << "element not found, was looking for " << height;
			return FinalizationPoint();
		}
	}

	// endregion
}}
