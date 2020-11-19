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
#include "BlockConsumers.h"
#include "catapult/disruptor/DisruptorConsumer.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace consumers {

	// functions in this file are used to implement validation consumers and are tested indirectly through those implementations

	/// Makes a block validation consumer that forwards all entity infos to \a process for validation.
	/// Validation will only be performed for entities for which \a requiresValidationPredicate returns \c true.
	disruptor::ConstBlockConsumer MakeBlockValidationConsumer(
			const RequiresValidationPredicate& requiresValidationPredicate,
			const std::function<validators::ValidationResult (const model::WeakEntityInfos&)>& process);

	/// Makes a transaction validation consumer that forwards all entity infos to \a process for validation.
	/// Each failure is forwarded to \a failedTransactionSink.
	disruptor::TransactionConsumer MakeTransactionValidationConsumer(
			const chain::FailedTransactionSink& failedTransactionSink,
			const std::function<std::vector<validators::ValidationResult> (model::WeakEntityInfos&)>& process);
}}
