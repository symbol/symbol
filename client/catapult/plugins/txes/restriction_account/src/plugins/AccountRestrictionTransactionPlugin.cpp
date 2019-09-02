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

#include "AccountRestrictionTransactionPlugin.h"
#include "src/model/AccountAddressRestrictionTransaction.h"
#include "src/model/AccountMosaicRestrictionTransaction.h"
#include "src/model/AccountOperationRestrictionTransaction.h"
#include "src/model/AccountRestrictionNotifications.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		struct AddressTraits {
			using UnresolvedValueType = UnresolvedAddress;
			using ResolvedValueType = Address;
			using ModifyAccountRestrictionNotification = ModifyAccountAddressRestrictionNotification;
			using ModifyAccountRestrictionValueNotification = ModifyAccountAddressRestrictionValueNotification;
		};

		struct MosaicTraits {
			using UnresolvedValueType = UnresolvedMosaicId;
			using ResolvedValueType = MosaicId;
			using ModifyAccountRestrictionNotification = ModifyAccountMosaicRestrictionNotification;
			using ModifyAccountRestrictionValueNotification = ModifyAccountMosaicRestrictionValueNotification;
		};

		struct OperationTraits {
			using UnresolvedValueType = model::EntityType;
			using ResolvedValueType = model::EntityType;
			using ModifyAccountRestrictionNotification = ModifyAccountOperationRestrictionNotification;
			using ModifyAccountRestrictionValueNotification = ModifyAccountOperationRestrictionValueNotification;
		};

		template<typename TTraits>
		class Publisher {
		public:
			template<typename TTransaction>
			static void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
				sub.notify(AccountRestrictionTypeNotification(transaction.RestrictionType));
				sub.notify(CreateAccountRestrictionModificationsNotification<TTransaction>(transaction));

				using ValueNotification = typename TTraits::ModifyAccountRestrictionValueNotification;
				const auto* pModifications = transaction.ModificationsPtr();
				for (auto i = 0u; i < transaction.ModificationsCount; ++i) {
					AccountRestrictionModification<typename TTraits::UnresolvedValueType> modification{
						pModifications[i].ModificationAction,
						pModifications[i].Value
					};
					sub.notify(ValueNotification(transaction.SignerPublicKey, transaction.RestrictionType, modification));
				}
			}

		private:
			template<typename TTransaction>
			static auto CreateAccountRestrictionModificationsNotification(const TTransaction& transaction) {
				return typename TTraits::ModifyAccountRestrictionNotification(
						transaction.SignerPublicKey,
						transaction.RestrictionType,
						transaction.ModificationsCount,
						transaction.ModificationsPtr());
			}
		};
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(AccountAddressRestriction, Default, Publisher<AddressTraits>::Publish)
	DEFINE_TRANSACTION_PLUGIN_FACTORY(AccountMosaicRestriction, Default, Publisher<MosaicTraits>::Publish)
	DEFINE_TRANSACTION_PLUGIN_FACTORY(AccountOperationRestriction, Default, Publisher<OperationTraits>::Publish)
}}
