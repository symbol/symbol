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

#include "ObserverContext.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace observers {

	// region NotifyMode

#define DEFINE_ENUM NotifyMode
#define ENUM_LIST NOTIFY_MODE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	// endregion

	// region ObserverState

	ObserverState::ObserverState(cache::CatapultCacheDelta& cache, state::CatapultState& state)
			: Cache(cache)
			, State(state)
			, pBlockStatementBuilder(nullptr)
	{}

	ObserverState::ObserverState(
			cache::CatapultCacheDelta& cache,
			state::CatapultState& state,
			model::BlockStatementBuilder& blockStatementBuilder)
			: Cache(cache)
			, State(state)
			, pBlockStatementBuilder(&blockStatementBuilder)
	{}

	// endregion

	// region ObserverContext

	namespace {
		model::ResolverContext BindConditional(
				const model::ResolverContext& resolvers,
				model::BlockStatementBuilder* pBlockStatementBuilder) {
			return pBlockStatementBuilder ? Bind(resolvers, *pBlockStatementBuilder) : resolvers;
		}

		ObserverStatementBuilder CreateObserverStatementBuilder(model::BlockStatementBuilder* pBlockStatementBuilder) {
			return pBlockStatementBuilder ? ObserverStatementBuilder(*pBlockStatementBuilder) : ObserverStatementBuilder();
		}
	}

	ObserverContext::ObserverContext(
			const ObserverState& state,
			catapult::Height height,
			NotifyMode mode,
			const model::ResolverContext& resolvers)
			: Cache(state.Cache)
			, State(state.State)
			, Height(height)
			, Mode(mode)
			, Resolvers(BindConditional(resolvers, state.pBlockStatementBuilder))
			, m_statementBuilder(CreateObserverStatementBuilder(state.pBlockStatementBuilder))
	{}

	ObserverStatementBuilder& ObserverContext::StatementBuilder() {
		return m_statementBuilder;
	}

	// endregion
}}
