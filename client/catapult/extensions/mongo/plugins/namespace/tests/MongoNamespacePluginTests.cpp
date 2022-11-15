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

#include "mongo/tests/test/MongoPluginTestUtils.h"
#include "plugins/txes/namespace/src/model/NamespaceEntityType.h"
#include "plugins/txes/namespace/src/model/NamespaceReceiptType.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct MongoNamespacePluginTraits {
		public:
			static constexpr auto RegisterSubsystem = RegisterMongoSubsystem;

			static std::vector<model::EntityType> GetTransactionTypes() {
				return {
					model::Entity_Type_Alias_Address,
					model::Entity_Type_Alias_Mosaic,
					model::Entity_Type_Namespace_Registration
				};
			}

			static std::vector<model::ReceiptType> GetReceiptTypes() {
				return {
					model::Receipt_Type_Namespace_Expired,
					model::Receipt_Type_Namespace_Deleted,
					model::Receipt_Type_Namespace_Rental_Fee
				};
			}

			static std::string GetStorageName() {
				return "{ NamespaceCache }";
			}
		};
	}

	DEFINE_MONGO_PLUGIN_TESTS(MongoNamespacePluginTests, MongoNamespacePluginTraits)
}}}
