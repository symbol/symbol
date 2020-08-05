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

	FileProofStorage::FileProofStorage(const std::string& dataDirectory)
			: m_dataDirectory(dataDirectory)
			, m_hashFile(m_dataDirectory, "proof.hashes")
			, m_indexFile((boost::filesystem::path(m_dataDirectory) / "proof.index.dat").generic_string())
			, m_heightIndexFile((boost::filesystem::path(m_dataDirectory) / "proof.height.dat").generic_string())
	{}

	FinalizationPoint FileProofStorage::finalizationPoint() const {
		return m_indexFile.exists() ? FinalizationPoint(m_indexFile.get()) : FinalizationPoint();
	}

	Height FileProofStorage::finalizedHeight() const {
		return m_heightIndexFile.exists() ? Height(m_heightIndexFile.get()) : Height();
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

	model::HeightHashPairRange FileProofStorage::loadFinalizedHashesFrom(FinalizationPoint point, size_t maxHashes) const {
		auto currentPoint = finalizationPoint();
		if (FinalizationPoint() == point || currentPoint < point)
			return model::HeightHashPairRange();

		auto numAvailableHashes = static_cast<size_t>((currentPoint - point).unwrap() + 1);
		auto numHashes = std::min(maxHashes, numAvailableHashes);
		return m_hashFile.loadRangeFrom(point, numHashes);
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
		auto currentPoint = finalizationPoint();

		if (currentPoint < point) {
			std::ostringstream out;
			out << "cannot load proof with point " << point << " when storage point is " << currentPoint;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		auto pProofFile = OpenProofFile(m_dataDirectory, point);
		return ReadPackedFinalizationProof(*pProofFile);
	}

	void FileProofStorage::saveProof(Height height, const FinalizationProof& proof) {
		if (proof.empty())
			CATAPULT_THROW_INVALID_ARGUMENT("cannot save empty proof");

		auto currentPoint = finalizationPoint();
		const auto& firstMessage = *proof.front();
		auto messagePoint = FinalizationPoint(firstMessage.StepIdentifier.Point);
		if (messagePoint != currentPoint + FinalizationPoint(1)) {
			std::ostringstream out;
			out << "cannot save proof with point " << messagePoint << " when storage point is " << currentPoint;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		auto currentHeight = finalizedHeight();
		if (currentHeight >= height) {
			std::ostringstream out;
			out << "cannot save proof with height " << height << " when storage height is " << currentHeight;
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

		m_hashFile.save(messagePoint, { height, *firstMessage.HashesPtr() });

		m_heightIndexFile.set(height.unwrap());
		m_indexFile.set(messagePoint.unwrap());
	}
}}
