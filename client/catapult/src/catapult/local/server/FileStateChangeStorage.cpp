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

#include "FileStateChangeStorage.h"
#include "catapult/cache/CacheChangesStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/subscribers/StateChangeInfo.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace local {

	namespace {
		void WriteChainScore(io::OutputStream& outputStream, const model::ChainScore& chainScore) {
			auto rawChainScore = chainScore.toArray();
			io::Write64(outputStream, rawChainScore[0]);
			io::Write64(outputStream, rawChainScore[1]);
		}

		class FileStateChangeStorage final : public subscribers::StateChangeSubscriber {
		public:
			FileStateChangeStorage(
					std::unique_ptr<io::OutputStream>&& pOutputStream,
					const supplier<CacheChangesStorages>& cacheChangesStoragesSupplier)
					: m_pOutputStream(std::move(pOutputStream))
					, m_cacheChangesStoragesSupplier(cacheChangesStoragesSupplier)
			{}

		public:
			void notifyScoreChange(const model::ChainScore& chainScore) override {
				write(subscribers::StateChangeOperationType::Score_Change);

				WriteChainScore(*m_pOutputStream, chainScore);
				m_pOutputStream->flush();
			}

			void notifyStateChange(const subscribers::StateChangeInfo& stateChangeInfo) override {
				write(subscribers::StateChangeOperationType::State_Change);

				io::Write(*m_pOutputStream, stateChangeInfo.ScoreDelta);
				io::Write(*m_pOutputStream, stateChangeInfo.Height);

				for (const auto& pStorage : m_cacheChangesStoragesSupplier())
					pStorage->saveAll(stateChangeInfo.CacheChanges, *m_pOutputStream);

				m_pOutputStream->flush();
			}

		private:
			void write(subscribers::StateChangeOperationType operationType) {
				io::Write8(*m_pOutputStream, utils::to_underlying_type(operationType));
			}

		private:
			std::unique_ptr<io::OutputStream> m_pOutputStream;
			supplier<CacheChangesStorages> m_cacheChangesStoragesSupplier;
		};
	}

	std::unique_ptr<subscribers::StateChangeSubscriber> CreateFileStateChangeStorage(
			std::unique_ptr<io::OutputStream>&& pOutputStream,
			const supplier<CacheChangesStorages>& cacheChangesStoragesSupplier) {
		return std::make_unique<FileStateChangeStorage>(std::move(pOutputStream), cacheChangesStoragesSupplier);
	}
}}
