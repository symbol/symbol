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

#include "MetadataTransactionPlugin.h"
#include "src/model/AccountMetadataTransaction.h"
#include "src/model/MetadataNotifications.h"
#include "src/model/MosaicMetadataTransaction.h"
#include "src/model/NamespaceMetadataTransaction.h"
#include "plugins/txes/namespace/src/model/NamespaceNotifications.h"
#include "catapult/model/Address.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		UnresolvedPartialMetadataKey ExtractPartialMetadataKey(const TTransaction& transaction, const PublishContext& context) {
			return { context.SignerAddress, transaction.TargetAddress, transaction.ScopedMetadataKey };
		}

		struct AccountTraits {
			template<typename TTransaction>
			static MetadataTarget ExtractMetadataTarget(const TTransaction&) {
				return { MetadataType::Account, 0 };
			}

			template<typename TTransaction>
			static void RaiseCustomNotifications(const TTransaction& transaction, NotificationSubscriber& sub) {
				sub.notify(AccountAddressNotification(transaction.TargetAddress));
			}
		};

		struct MosaicTraits {
			template<typename TTransaction>
			static MetadataTarget ExtractMetadataTarget(const TTransaction& transaction) {
				return { MetadataType::Mosaic, transaction.TargetMosaicId.unwrap() };
			}

			template<typename TTransaction>
			static void RaiseCustomNotifications(const TTransaction& transaction, NotificationSubscriber& sub) {
				sub.notify(MosaicRequiredNotification(transaction.TargetAddress, transaction.TargetMosaicId));
			}
		};

		struct NamespaceTraits {
			template<typename TTransaction>
			static MetadataTarget ExtractMetadataTarget(const TTransaction& transaction) {
				return { MetadataType::Namespace, transaction.TargetNamespaceId.unwrap() };
			}

			template<typename TTransaction>
			static void RaiseCustomNotifications(const TTransaction& transaction, NotificationSubscriber& sub) {
				sub.notify(NamespaceRequiredNotification(transaction.TargetAddress, transaction.TargetNamespaceId));
			}
		};

		template<typename TTraits>
		class Publisher {
		public:
			template<typename TTransaction>
			static void Publish(const TTransaction& transaction, const PublishContext& context, NotificationSubscriber& sub) {
				sub.notify(MetadataSizesNotification(transaction.ValueSizeDelta, transaction.ValueSize));
				sub.notify(MetadataValueNotification(
						ExtractPartialMetadataKey(transaction, context),
						TTraits::ExtractMetadataTarget(transaction),
						transaction.ValueSizeDelta,
						transaction.ValueSize,
						transaction.ValuePtr()));

				TTraits::RaiseCustomNotifications(transaction, sub);
			}
		};
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(AccountMetadata, Only_Embeddable, Publisher<AccountTraits>::Publish)
	DEFINE_TRANSACTION_PLUGIN_FACTORY(MosaicMetadata, Only_Embeddable, Publisher<MosaicTraits>::Publish)
	DEFINE_TRANSACTION_PLUGIN_FACTORY(NamespaceMetadata, Only_Embeddable, Publisher<NamespaceTraits>::Publish)
}}
