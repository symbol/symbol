#include "RegisterNamespaceTransactionPlugin.h"
#include "src/model/NamespaceNotifications.h"
#include "src/model/RegisterNamespaceTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"
#include "catapult/constants.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		static void PublishBalanceTransfer(
				const NamespaceRentalFeeConfiguration& config,
				const TTransaction& transaction,
				NotificationSubscriber& sub) {
			// a. exempt the nemesis account
			if (config.NemesisPublicKey == transaction.Signer)
				return;

			auto rentalFee = config.ChildFee;
			if (transaction.IsRootRegistration()) {
				// b. don't charge fees for eternal namespaces
				if (Eternal_Artifact_Duration == transaction.Duration)
					return;

				rentalFee = Amount(config.RootFeePerBlock.unwrap() * transaction.Duration.unwrap());
			}

			sub.notify(BalanceTransferNotification(transaction.Signer, config.SinkAddress, Xem_Id, rentalFee));
		}

		template<typename TTransaction>
		auto CreatePublisher(const NamespaceRentalFeeConfiguration& config) {
			return [config](const TTransaction& transaction, NotificationSubscriber& sub) {
				// 1. sink account notification
				sub.notify(AccountPublicKeyNotification(config.SinkPublicKey));

				// 2. rental fee charge
				PublishBalanceTransfer(config, transaction, sub);

				// 3. registration notifications
				sub.notify(NamespaceNotification(transaction.NamespaceType));
				auto parentId = Namespace_Base_Id;
				if (transaction.IsRootRegistration()) {
					using Notification = RootNamespaceNotification;
					sub.notify(Notification(transaction.Signer, transaction.NamespaceId, transaction.Duration));
				} else {
					using Notification = ChildNamespaceNotification;
					sub.notify(Notification(transaction.Signer, transaction.NamespaceId, transaction.ParentId));
					parentId = transaction.ParentId;
				}

				sub.notify(NamespaceNameNotification(
						transaction.NamespaceId,
						parentId,
						transaction.NamespaceNameSize,
						transaction.NamePtr()));
			};
		}
	}

	std::unique_ptr<TransactionPlugin> CreateRegisterNamespaceTransactionPlugin(const NamespaceRentalFeeConfiguration& config) {
		return TransactionPluginFactory::Create<RegisterNamespaceTransaction, EmbeddedRegisterNamespaceTransaction>(
				CreatePublisher<RegisterNamespaceTransaction>(config),
				CreatePublisher<EmbeddedRegisterNamespaceTransaction>(config));
	}
}}
