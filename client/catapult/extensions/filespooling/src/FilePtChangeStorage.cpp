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

#include "FilePtChangeStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/TransactionInfoSerializer.h"
#include "catapult/model/Cosignature.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"

namespace catapult { namespace filespooling {

	namespace {
		class FilePtChangeStorage final : public cache::PtChangeSubscriber {
		public:
			explicit FilePtChangeStorage(std::unique_ptr<io::OutputStream>&& pOutputStream)
					: m_pOutputStream(std::move(pOutputStream))
			{}

		public:
			void notifyAddPartials(const TransactionInfos& transactionInfos) override {
				saveInfos(subscribers::PtChangeOperationType::Add_Partials, transactionInfos);
			}

			void notifyRemovePartials(const TransactionInfos& transactionInfos) override {
				saveInfos(subscribers::PtChangeOperationType::Remove_Partials, transactionInfos);
			}

			void notifyAddCosignature(
					const model::TransactionInfo& parentTransactionInfo,
					const model::Cosignature& cosignature) override {
				io::Write8(*m_pOutputStream, utils::to_underlying_type(subscribers::PtChangeOperationType::Add_Cosignature));
				m_pOutputStream->write({ reinterpret_cast<const uint8_t*>(&cosignature), sizeof(model::Cosignature) });
				io::WriteTransactionInfo(parentTransactionInfo, *m_pOutputStream);
			}

			void flush() override {
				m_pOutputStream->flush();
			}

		private:
			void saveInfos(subscribers::PtChangeOperationType operationType, const TransactionInfos& transactionInfos) {
				io::Write8(*m_pOutputStream, utils::to_underlying_type(operationType));
				io::WriteTransactionInfos(transactionInfos, *m_pOutputStream);
			}

		private:
			std::unique_ptr<io::OutputStream> m_pOutputStream;
		};
	}

	std::unique_ptr<cache::PtChangeSubscriber> CreateFilePtChangeStorage(std::unique_ptr<io::OutputStream>&& pOutputStream) {
		return std::make_unique<FilePtChangeStorage>(std::move(pOutputStream));
	}
}}
