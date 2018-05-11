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

#include "PluginUtils.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "catapult/observers/ReverseNotificationObserverAdapter.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/validators/AggregateEntityValidator.h"
#include "catapult/validators/NotificationValidatorAdapter.h"

namespace catapult { namespace extensions {

	namespace {
		template<typename TAdapter, typename TAdaptee>
		auto MakeAdapter(const plugins::PluginManager& manager, std::unique_ptr<TAdaptee>&& pAdaptee) {
			return std::make_unique<TAdapter>(std::move(pAdaptee), manager.createNotificationPublisher());
		}
	}

	std::unique_ptr<const validators::stateless::AggregateEntityValidator> CreateStatelessValidator(
			const plugins::PluginManager& manager) {
		// create an aggregate entity validator of one
		auto validators = validators::ValidatorVectorT<>();
		validators.push_back(MakeAdapter<validators::NotificationValidatorAdapter>(manager, manager.createStatelessValidator()));
		return std::make_unique<validators::stateless::AggregateEntityValidator>(std::move(validators));
	}

	std::unique_ptr<const observers::EntityObserver> CreateEntityObserver(const plugins::PluginManager& manager) {
		return MakeAdapter<observers::NotificationObserverAdapter>(manager, manager.createObserver());
	}

	std::unique_ptr<const observers::EntityObserver> CreateUndoEntityObserver(const plugins::PluginManager& manager) {
		return MakeAdapter<observers::ReverseNotificationObserverAdapter>(manager, manager.createObserver());
	}

	std::unique_ptr<const observers::EntityObserver> CreatePermanentEntityObserver(const plugins::PluginManager& manager) {
		return MakeAdapter<observers::NotificationObserverAdapter>(manager, manager.createPermanentObserver());
	}
}}
