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

#include "src/plugins/AddressAliasTransactionPlugin.h"
#include "src/model/AddressAliasTransaction.h"
#include "tests/test/AliasTransactionPluginTests.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS AddressAliasTransactionPluginTests

	// region TransactionPlugin

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(AddressAlias, 1, 1,)

		struct NotificationTraits {
		public:
			using Notification_Type = model::AliasedAddressNotification;

		public:
			static constexpr size_t NumNotifications() {
				return 2u;
			}

		public:
			template<typename TTransaction>
			static auto& TransactionAlias(TTransaction& transaction) {
				return transaction.Address;
			}
		};
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Alias_Address)

	DEFINE_ALIAS_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, AddressAlias, NotificationTraits)

	// endregion
}}
