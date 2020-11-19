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
#include "catapult/model/Elements.h"
#include "catapult/observers/ObserverTypes.h"
#include <memory>

namespace catapult { namespace model { struct Block; } }

namespace catapult { namespace chain {

	/// Block execution context.
	struct BlockExecutionContext {
	public:
		/// Creates a block execution context around \a observer, \a resolvers and \a state.
		BlockExecutionContext(
				const observers::EntityObserver& observer,
				const model::ResolverContext& resolvers,
				observers::ObserverState& state)
				: Observer(observer)
				, Resolvers(resolvers)
				, State(state)
		{}

	public:
		/// Observer to execute the block.
		const observers::EntityObserver& Observer;

		/// Alias resolvers.
		const model::ResolverContext& Resolvers;

		/// State to update during observation.
		observers::ObserverState& State;
	};

	/// Executes \a blockElement using the specified execution context (\a executionContext).
	void ExecuteBlock(const model::BlockElement& blockElement, const BlockExecutionContext& executionContext);

	/// Rollbacks \a blockElement using the specified execution context (\a executionContext).
	void RollbackBlock(const model::BlockElement& blockElement, const BlockExecutionContext& executionContext);
}}
