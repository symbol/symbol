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

#include "src/plugins/AccountLinkPlugin.h"
#include "plugins/txes/accountlink/src/model/AccountLinkEntityType.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct AccountLinkPluginTraits : public test::EmptyPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				PluginManager manager(model::BlockChainConfiguration::Uninitialized(), StorageConfiguration());
				RegisterAccountLinkSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Account_Link };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "AccountLinkActionValidator" };
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {
					"AccountLinkAvailabilityValidator",
					"NewRemoteAccountAvailabilityValidator",
					"RemoteSenderValidator",
					"RemoteInteractionValidator"
				};
			}

			static std::vector<std::string> GetObserverNames() {
				return { "AccountLinkObserver" };
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(AccountLinkPluginTests, AccountLinkPluginTraits)
}}
