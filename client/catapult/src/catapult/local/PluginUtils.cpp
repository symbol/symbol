#include "PluginUtils.h"
#include "NotificationObserverAdapter.h"
#include "NotificationValidatorAdapter.h"
#include "ReverseNotificationObserverAdapter.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/validators/AggregateEntityValidator.h"

namespace catapult { namespace local {
	std::unique_ptr<const validators::stateless::AggregateEntityValidator> CreateStatelessValidator(
			const plugins::PluginManager& manager) {
		// create an aggregate entity validator of one
		auto validators = validators::ValidatorVectorT<>();
		validators.push_back(std::make_unique<NotificationValidatorAdapter>(
				manager.transactionRegistry(),
				manager.createStatelessValidator()));
		return std::make_unique<validators::stateless::AggregateEntityValidator>(std::move(validators));
	}

	std::unique_ptr<const observers::EntityObserver> CreateEntityObserver(const plugins::PluginManager& manager) {
		return std::make_unique<NotificationObserverAdapter>(manager.transactionRegistry(), manager.createObserver());
	}

	std::unique_ptr<const observers::EntityObserver> CreateUndoEntityObserver(const plugins::PluginManager& manager) {
		return std::make_unique<ReverseNotificationObserverAdapter>(manager.transactionRegistry(), manager.createObserver());
	}

	std::unique_ptr<const observers::EntityObserver> CreatePermanentEntityObserver(const plugins::PluginManager& manager) {
		return std::make_unique<NotificationObserverAdapter>(manager.transactionRegistry(), manager.createPermanentObserver());
	}
}}
