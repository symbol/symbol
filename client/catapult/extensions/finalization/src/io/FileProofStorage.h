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

#pragma once
#include "ProofStorage.h"
#include "catapult/io/FixedSizeValueStorage.h"
#include "catapult/io/IndexFile.h"
#include <string>

namespace catapult { namespace io {

	/// File-based proof storage.
	class FileProofStorage final : public ProofStorage {
	public:
		/// Creates a file-based proof storage, where proofs will be stored inside \a dataDirectory.
		explicit FileProofStorage(const std::string& dataDirectory);

	public:
		model::FinalizationStatistics statistics() const override;
		std::shared_ptr<const model::FinalizationProof> loadProof(FinalizationPoint point) const override;
		std::shared_ptr<const model::FinalizationProof> loadProof(Height point) const override;
		void saveProof(const model::FinalizationProof& proof) override;

	private:
		FinalizationPoint findPointForHeight(Height height) const;

	private:
		class FinalizationIndexFile {
		public:
			explicit FinalizationIndexFile(const std::string& filename, LockMode lockMode = LockMode::File);

		public:
			bool exists() const;

			model::FinalizationStatistics get() const;

		public:
			void set(const model::FinalizationStatistics& finalizationStatistics);

		private:
			RawFile open(OpenMode mode) const;

		private:
			std::string m_filename;
			LockMode m_lockMode;
		};

	private:
		std::string m_dataDirectory;
		FinalizationPointHeightFile m_pointHeightMapping;
		FinalizationIndexFile m_indexFile;
	};
}}
