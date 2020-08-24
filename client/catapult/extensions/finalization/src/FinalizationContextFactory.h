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

#pragma once
#include "FinalizationConfiguration.h"
#include "finalization/src/model/FinalizationContext.h"

namespace catapult {
	namespace cache { class AccountStateCache; }
	namespace extensions { class ServiceState; }
	namespace io { class BlockStorageCache; }
}

namespace catapult { namespace finalization {

	/// Factory for creating finalization contexts.
	class FinalizationContextFactory {
	public:
		/// Creates a factory given \a config and \a state.
		FinalizationContextFactory(const FinalizationConfiguration& config, const extensions::ServiceState& state);

	public:
		/// Creates a finalization context for \a point at \a height.
		model::FinalizationContext create(FinalizationPoint point, Height height) const;

	private:
		FinalizationConfiguration m_config;
		const cache::AccountStateCache& m_accountStateCache;
		const io::BlockStorageCache& m_blockStorage;
	};
}}
