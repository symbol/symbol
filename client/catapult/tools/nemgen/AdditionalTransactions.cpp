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

#include "AdditionalTransactions.h"
#include "NemesisConfiguration.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/RawFile.h"
#include "catapult/model/AggregateEntityType.h"
#include "catapult/model/AggregateNotifications.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransferEntityType.h"
#include "catapult/utils/MemoryUtils.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		// region AdditionalTransactionNotificationSubscriber

		class AdditionalTransactionNotificationSubscriber : public model::NotificationSubscriber {
		public:
			AdditionalTransactionNotificationSubscriber(const NemesisConfiguration& config, std::vector<std::string>& violations)
					: m_violations(violations)
					, m_nemesisSignerAddress(GetNemesisSignerAddress(config))
			{}

		public:
			void notify(const model::Notification& notification) override {
				if (model::Core_Transaction_Notification == notification.Type) {
					using NotificationType = model::TransactionNotification;
					checkTransactionType(static_cast<const NotificationType&>(notification).TransactionType);
				}

				if (model::Aggregate_Embedded_Transaction_Notification == notification.Type) {
					using NotificationType = model::AggregateEmbeddedTransactionNotification;
					checkTransactionType(static_cast<const NotificationType&>(notification).Transaction.Type);
				}

				if (model::Core_Balance_Transfer_Notification == notification.Type) {
					using NotificationType = model::BalanceTransferNotification;
					checkSender(static_cast<const NotificationType&>(notification).Sender);
				}
			}

		private:
			void checkTransactionType(model::EntityType transactionType) {
				if (model::Entity_Type_Transfer == transactionType)
					m_violations.push_back("Transfer is not supported as additional transaction");

				if (model::Entity_Type_Aggregate_Bonded == transactionType)
					m_violations.push_back("Aggregate_Bonded is not supported as additional transaction");
			}

			void checkSender(const Address& senderAddress) {
				if (m_nemesisSignerAddress == senderAddress)
					m_violations.push_back("Nemesis Signer cannot sign any additional transaction");
			}

		private:
			std::vector<std::string>& m_violations;
			Address m_nemesisSignerAddress;
		};

		// endregion

		// region utils

		auto GetSortedPathNames(const std::string& directoryPath) {
			std::vector<std::string> pathNames;

			if (directoryPath.empty())
				return pathNames;

			auto begin = boost::filesystem::directory_iterator(directoryPath);
			auto end = boost::filesystem::directory_iterator();
			for (auto iter = begin; end != iter; ++iter) {
				auto pathName = iter->path().generic_string();
				pathNames.insert(std::upper_bound(pathNames.begin(), pathNames.end(), pathName), pathName);
			}

			return pathNames;
		}

		auto LoadTransaction(const std::string& filePath) {
			io::RawFile txFile(filePath, io::OpenMode::Read_Only);
			auto transactionSize = io::Read32(txFile);
			auto pTransaction = utils::MakeSharedWithSize<model::Transaction>(transactionSize);
			pTransaction->Size = transactionSize;
			txFile.read({ reinterpret_cast<uint8_t*>(pTransaction.get()) + sizeof(uint32_t), transactionSize - sizeof(uint32_t) });
			if (txFile.size() != txFile.position())
				CATAPULT_THROW_RUNTIME_ERROR_1("transaction file has invalid size", filePath);

			return pTransaction;
		}

		std::vector<std::string> ValidateAdditionalTransaction(
				const NemesisConfiguration& config,
				const model::NotificationPublisher& notificationPublisher,
				const model::Transaction& transaction) {
			std::vector<std::string> violations;
			if (Timestamp(1) != transaction.Deadline)
				violations.push_back("nemesis transactions need to have deadline set to 1");

			if (Amount(0) != transaction.MaxFee)
				violations.push_back("nemesis transactions need to have max fee set to 0");

			Hash256 zeroHash;
			AdditionalTransactionNotificationSubscriber subscriber(config, violations);
			notificationPublisher.publish({ transaction, zeroHash }, subscriber);
			return violations;
		}

		std::string FormatViolations(const std::vector<std::string>& violations, const std::string& filePath) {
			std::ostringstream out;
			out << "found violations for " << filePath;
			for (const auto& violation : violations)
				out << std::endl << " + " << violation;

			return out.str();
		}

		// endregion
	}

	model::Transactions LoadAndValidateAdditionalTransactions(
			const NemesisConfiguration& config,
			const model::NotificationPublisher& notificationPublisher) {
		bool hasViolations = false;
		model::Transactions transactions;
		auto pathNames = GetSortedPathNames(config.TransactionsDirectory);
		for (const auto& filePath : pathNames) {
			auto pTransaction = LoadTransaction(filePath);
			transactions.push_back(std::move(pTransaction));

			auto violations = ValidateAdditionalTransaction(config, notificationPublisher, *transactions.back());
			if (!violations.empty()) {
				CATAPULT_LOG(warning) << FormatViolations(violations, filePath);
				hasViolations = true;
			}
		}

		if (hasViolations)
			CATAPULT_THROW_RUNTIME_ERROR("one or more additional transactions failed validation");

		CATAPULT_LOG(info) << "loaded " << transactions.size() << " additional transactions";
		return transactions;
	}
}}}
