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

#include "SecretProofTransactionPlugin.h"
#include "src/model/SecretLockNotifications.h"
#include "src/model/SecretProofTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult {
namespace plugins {

	namespace {
		template <typename TTransaction>
		void Publish(const TTransaction& transaction, const PublishContext& context, NotificationSubscriber& sub) {
			sub.notify(SecretLockHashAlgorithmNotification(transaction.HashAlgorithm));
			sub.notify(ProofSecretNotification(
				transaction.HashAlgorithm,
				transaction.Secret,
				{ transaction.ProofPtr(), transaction.ProofSize }));
			sub.notify(ProofPublicationNotification(
				context.SignerAddress,
				transaction.HashAlgorithm,
				transaction.Secret,
				transaction.RecipientAddress));
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(SecretProof, Default, Publish)
}
}
