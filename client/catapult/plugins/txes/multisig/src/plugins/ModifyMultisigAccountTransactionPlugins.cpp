#include "ModifyMultisigAccountTransactionPlugins.h"
#include "src/model/ModifyMultisigAccountTransaction.h"
#include "src/model/MultisigNotifications.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
			// 1. cosig changes
			if (0 < transaction.ModificationsCount) {
				// raise new cosigner notifications first because they are used for multisig loop detection
				const auto* pModifications = transaction.ModificationsPtr();
				for (auto i = 0u; i < transaction.ModificationsCount; ++i) {
					if (model::CosignatoryModificationType::Add == pModifications[i].ModificationType)
						sub.notify(ModifyMultisigNewCosignerNotification(transaction.Signer, pModifications[i].CosignatoryPublicKey));
				}

				sub.notify(ModifyMultisigCosignersNotification(transaction.Signer, transaction.ModificationsCount, pModifications));
			}

			// 2. setting changes
			sub.notify(ModifyMultisigSettingsNotification(
					transaction.Signer,
					transaction.MinRemovalDelta,
					transaction.MinApprovalDelta));
		}
	}

	std::unique_ptr<TransactionPlugin> CreateModifyMultisigAccountTransactionPlugin() {
		return TransactionPluginFactory::Create<ModifyMultisigAccountTransaction, EmbeddedModifyMultisigAccountTransaction>(
				Publish<ModifyMultisigAccountTransaction>,
				Publish<EmbeddedModifyMultisigAccountTransaction>);
	}
}}
