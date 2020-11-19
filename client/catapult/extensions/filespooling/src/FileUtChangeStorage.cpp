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

#include "FileUtChangeStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/TransactionInfoSerializer.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"

namespace catapult { namespace filespooling {

	namespace {
		class FileUtChangeStorage final : public cache::UtChangeSubscriber {
		public:
			explicit FileUtChangeStorage(std::unique_ptr<io::OutputStream>&& pOutputStream)
					: m_pOutputStream(std::move(pOutputStream))
			{}

		public:
			void notifyAdds(const TransactionInfos& transactionInfos) override {
				saveInfos(subscribers::UtChangeOperationType::Add, transactionInfos);
			}

			void notifyRemoves(const TransactionInfos& transactionInfos) override {
				saveInfos(subscribers::UtChangeOperationType::Remove, transactionInfos);
			}

			void flush() override {
				m_pOutputStream->flush();
			}

		private:
			void saveInfos(subscribers::UtChangeOperationType operationType, const TransactionInfos& transactionInfos) {
				io::Write8(*m_pOutputStream, utils::to_underlying_type(operationType));
				io::WriteTransactionInfos(transactionInfos, *m_pOutputStream);
			}

		private:
			std::unique_ptr<io::OutputStream> m_pOutputStream;
		};
	}

	std::unique_ptr<cache::UtChangeSubscriber> CreateFileUtChangeStorage(std::unique_ptr<io::OutputStream>&& pOutputStream) {
		return std::make_unique<FileUtChangeStorage>(std::move(pOutputStream));
	}
}}
