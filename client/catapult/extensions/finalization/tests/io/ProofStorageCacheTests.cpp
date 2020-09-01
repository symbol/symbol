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

#include "finalization/src/io/ProofStorageCache.h"
#include "finalization/src/io/FileProofStorage.h"
#include "finalization/tests/test/ProofStorageTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS ProofStorageCacheTests

	namespace {
		// wraps a ProofStorageCache in a ProofStorage so that it can be tested via the tests in ProofStorageTests.h
		class ProofStorageCacheToProofStorageAdapter : public ProofStorage {
		public:
			explicit ProofStorageCacheToProofStorageAdapter(std::unique_ptr<ProofStorage>&& pStorage) : m_cache(std::move(pStorage))
			{}

		public:
			model::FinalizationStatistics statistics() const override {
				return m_cache.view().statistics();
			}

			std::shared_ptr<const model::FinalizationProof> loadProof(FinalizationPoint point) const override {
				return m_cache.view().loadProof(point);
			}

			std::shared_ptr<const model::FinalizationProof> loadProof(Height height) const override {
				return m_cache.view().loadProof(height);
			}

			void saveProof(const model::FinalizationProof& proof) override {
				m_cache.modifier().saveProof(proof);
			}

		private:
			ProofStorageCache m_cache;
		};

		struct ProofStorageCacheTraits {
			static std::unique_ptr<ProofStorage> CreateStorage(const std::string& destination) {
				return std::make_unique<ProofStorageCacheToProofStorageAdapter>(std::make_unique<FileProofStorage>(destination));
			}
		};
	}

	DEFINE_PROOF_STORAGE_TESTS(ProofStorageCacheTraits)
}}
