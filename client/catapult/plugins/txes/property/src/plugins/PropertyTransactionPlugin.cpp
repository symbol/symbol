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

#include "PropertyTransactionPlugin.h"
#include "src/model/AddressPropertyTransaction.h"
#include "src/model/MosaicPropertyTransaction.h"
#include "src/model/PropertyNotifications.h"
#include "src/model/TransactionTypePropertyTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		struct AddressTraits {
			using UnresolvedValueType = UnresolvedAddress;
			using ResolvedValueType = Address;
			using ModifyPropertyNotification = ModifyAddressPropertyNotification;
			using ModifyPropertyValueNotification = ModifyAddressPropertyValueNotification;
		};

		struct MosaicTraits {
			using UnresolvedValueType = UnresolvedMosaicId;
			using ResolvedValueType = MosaicId;
			using ModifyPropertyNotification = ModifyMosaicPropertyNotification;
			using ModifyPropertyValueNotification = ModifyMosaicPropertyValueNotification;
		};

		struct TransactionTypeTraits {
			using UnresolvedValueType = model::EntityType;
			using ResolvedValueType = model::EntityType;
			using ModifyPropertyNotification = ModifyTransactionTypePropertyNotification;
			using ModifyPropertyValueNotification = ModifyTransactionTypePropertyValueNotification;
		};

		template<typename TTraits>
		class Publisher {
		public:
			template<typename TTransaction>
			static void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
				sub.notify(PropertyTypeNotification(transaction.PropertyType));
				sub.notify(CreatePropertyModificationsNotification<TTransaction>(transaction));

				using ValueNotification = typename TTraits::ModifyPropertyValueNotification;
				const auto* pModifications = transaction.ModificationsPtr();
				for (auto i = 0u; i < transaction.ModificationsCount; ++i) {
					PropertyModification<typename TTraits::UnresolvedValueType> modification{
						pModifications[i].ModificationType,
						pModifications[i].Value
					};
					sub.notify(ValueNotification(transaction.Signer, transaction.PropertyType, modification));
				}
			}

		private:
			template<typename TTransaction>
			static auto CreatePropertyModificationsNotification(const TTransaction& transaction) {
				return typename TTraits::ModifyPropertyNotification(
						transaction.Signer,
						transaction.PropertyType,
						transaction.ModificationsCount,
						transaction.ModificationsPtr());
			}
		};
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(AddressProperty, Default, Publisher<AddressTraits>::Publish)
	DEFINE_TRANSACTION_PLUGIN_FACTORY(MosaicProperty, Default, Publisher<MosaicTraits>::Publish)
	DEFINE_TRANSACTION_PLUGIN_FACTORY(TransactionTypeProperty, Default, Publisher<TransactionTypeTraits>::Publish)
}}
