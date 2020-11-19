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

#include "src/FileBlockChangeStorage.h"
#include "src/FileFinalizationStorage.h"
#include "src/FilePtChangeStorage.h"
#include "src/FileTransactionStatusStorage.h"
#include "src/FileUtChangeStorage.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/io/FileQueue.h"

namespace catapult { namespace filespooling {

	namespace {
		class FileQueueFactory {
		public:
			explicit FileQueueFactory(const std::string& dataDirectory)
					: m_dataDirectory(config::CatapultDataDirectoryPreparer::Prepare(dataDirectory))
			{}

		public:
			std::unique_ptr<io::FileQueueWriter> create(const std::string& queueName) const {
				return std::make_unique<io::FileQueueWriter>(m_dataDirectory.spoolDir(queueName).str());
			}

		private:
			config::CatapultDataDirectory m_dataDirectory;
		};

		void RegisterExtension(extensions::ProcessBootstrapper& bootstrapper) {
			// register subscribers
			FileQueueFactory factory(bootstrapper.config().User.DataDirectory);
			auto& subscriptionManager = bootstrapper.subscriptionManager();
			subscriptionManager.addBlockChangeSubscriber(CreateFileBlockChangeStorage(factory.create("block_change")));
			subscriptionManager.addUtChangeSubscriber(CreateFileUtChangeStorage(factory.create("unconfirmed_transactions_change")));
			subscriptionManager.addPtChangeSubscriber(CreateFilePtChangeStorage(factory.create("partial_transactions_change")));
			subscriptionManager.addFinalizationSubscriber(CreateFileFinalizationStorage(factory.create("finalization")));
			subscriptionManager.addTransactionStatusSubscriber(CreateFileTransactionStatusStorage(factory.create("transaction_status")));
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::ProcessBootstrapper& bootstrapper) {
	catapult::filespooling::RegisterExtension(bootstrapper);
}
