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

#pragma once
#include "catapult/model/EntityType.h"
#include "catapult/plugins.h"
#include <memory>

namespace catapult {
	namespace model {
		class TransactionPlugin;
		class TransactionRegistry;
	}
	namespace utils { class TimeSpan; }
}

namespace catapult { namespace plugins {

	/// Creates an aggregate transaction plugin around \a transactionRegistry for transactions with type \a transactionType.
	PLUGIN_API
	std::unique_ptr<model::TransactionPlugin> CreateAggregateTransactionPlugin(
			const model::TransactionRegistry& transactionRegistry,
			model::EntityType transactionType);

	/// Creates an aggregate transaction plugin around \a transactionRegistry for transactions with type \a transactionType
	/// and specified max transaction lifetime (\a maxTransactionLifetime).
	PLUGIN_API
	std::unique_ptr<model::TransactionPlugin> CreateAggregateTransactionPlugin(
			const model::TransactionRegistry& transactionRegistry,
			const utils::TimeSpan& maxTransactionLifetime,
			model::EntityType transactionType);
}}
