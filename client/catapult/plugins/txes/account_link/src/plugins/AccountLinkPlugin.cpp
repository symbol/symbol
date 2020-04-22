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

#include "AccountLinkPlugin.h"
#include "AccountLinkTransactionPlugin.h"
#include "src/observers/Observers.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterAccountLinkSubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateAccountLinkTransactionPlugin());

		manager.addStatefulValidatorHook([](auto& builder) {
			builder
				.add(validators::CreateAccountLinkAvailabilityValidator())
				.add(validators::CreateNewRemoteAccountAvailabilityValidator())
				.add(validators::CreateRemoteSenderValidator())
				.add(validators::CreateRemoteInteractionValidator());
		});

		manager.addObserverHook([](auto& builder) {
			builder.add(observers::CreateAccountLinkObserver());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterAccountLinkSubsystem(manager);
}
