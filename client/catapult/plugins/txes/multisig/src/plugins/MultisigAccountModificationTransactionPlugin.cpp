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

#include "MultisigAccountModificationTransactionPlugin.h"
#include "src/model/MultisigAccountModificationTransaction.h"
#include "src/model/MultisigNotifications.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		void Publish(const TTransaction& transaction, const PublishContext&, NotificationSubscriber& sub) {
			// 1. basic
			sub.notify(InternalPaddingNotification(transaction.MultisigAccountModificationTransactionBody_Reserved1));

			// 2. cosig changes
			utils::KeySet addedCosignatoryKeys;
			if (0 < transaction.PublicKeyAdditionsCount || 0 < transaction.PublicKeyDeletionsCount) {
				// - raise new cosignatory notifications first because they are used for multisig loop detection
				// - notify cosignatories' public keys in order to allow added cosignatories to get aggregate notifications
				const auto* pPublicKeyAdditions = transaction.PublicKeyAdditionsPtr();
				for (auto i = 0u; i < transaction.PublicKeyAdditionsCount; ++i) {
					sub.notify(AccountPublicKeyNotification(pPublicKeyAdditions[i]));
					sub.notify(MultisigNewCosignatoryNotification(transaction.SignerPublicKey, pPublicKeyAdditions[i]));
					addedCosignatoryKeys.insert(pPublicKeyAdditions[i]);
				}

				sub.notify(MultisigCosignatoriesNotification(
						transaction.SignerPublicKey,
						transaction.PublicKeyAdditionsCount,
						pPublicKeyAdditions,
						transaction.PublicKeyDeletionsCount,
						transaction.PublicKeyDeletionsPtr()));
			}

			if (!addedCosignatoryKeys.empty())
				sub.notify(AddressInteractionNotification(transaction.SignerPublicKey, transaction.Type, {}, addedCosignatoryKeys));

			// 3. setting changes
			sub.notify(MultisigSettingsNotification(
					transaction.SignerPublicKey,
					transaction.MinRemovalDelta,
					transaction.MinApprovalDelta));
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(MultisigAccountModification, Only_Embeddable, Publish)
}}
