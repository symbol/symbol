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

#include "RepairState.h"
#include "catapult/cache/CacheChangesStorage.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/subscribers/BrokerMessageReaders.h"
#include "catapult/subscribers/StateChangeReader.h"
#include "catapult/subscribers/StateChangeSubscriber.h"

namespace catapult { namespace local {

	namespace {
		// region StateRepairer

		class StateRepairer {
		public:
			StateRepairer(const config::CatapultDirectory& stateChangeDirectory, const cache::CatapultCache& catapultCache)
					: m_stateChangeDirectory(stateChangeDirectory)
					, m_catapultCache(catapultCache)
					, m_isBrokerRecovery(std::filesystem::exists(m_stateChangeDirectory.file("index_broker_r.dat")))
			{}

		public:
			bool isBrokerRecovery() const {
				return m_isBrokerRecovery;
			}

		public:
			void readAll(
					const std::string& indexReaderFilename,
					const std::string& indexWriterFilename,
					subscribers::StateChangeSubscriber& stateChangeSubscriber) {
				subscribers::ReadAll(
						{ m_stateChangeDirectory.str(), indexReaderFilename, indexWriterFilename },
						stateChangeSubscriber,
						[&catapultCache = m_catapultCache](auto& inputStream, auto& subscriber) {
							return subscribers::ReadNextStateChange(inputStream, catapultCache.changesStorages(), subscriber);
						});
			}

			void reindex(const std::string& destinationIndexFilename, const std::string& sourceIndexFilename) {
				io::IndexFile destinationIndexFile(m_stateChangeDirectory.file(destinationIndexFilename));
				io::IndexFile sourceIndexFile(m_stateChangeDirectory.file(sourceIndexFilename));
				destinationIndexFile.set(sourceIndexFile.get());
			}

		private:
			config::CatapultDirectory m_stateChangeDirectory;
			const cache::CatapultCache& m_catapultCache;
			bool m_isBrokerRecovery;
		};

		// endregion
	}

	void RepairState(
			const config::CatapultDirectory& stateChangeDirectory,
			const cache::CatapultCache& catapultCache,
			subscribers::StateChangeSubscriber& registeredSubscriber,
			subscribers::StateChangeSubscriber& repairSubscriber) {
		StateRepairer repairer(stateChangeDirectory, catapultCache);
		auto finalIndexReaderFilename = repairer.isBrokerRecovery() ? "index_broker_r.dat" : "index_server_r.dat";

		// 1. catch up registered subscribers
		repairer.readAll(finalIndexReaderFilename, "index.dat", registeredSubscriber);

		// 2. repair and forward to registered subscribers
		repairer.readAll("index.dat", "index_server.dat", repairSubscriber);

		// 3. fixup final reader index
		repairer.reindex(finalIndexReaderFilename, "index.dat");
	}
}}
