#include "TransferPlugin.h"
#include "TransferTransactionPlugins.h"
#include "src/config/TransferConfiguration.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterTransferSubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateTransferTransactionPlugin());

		auto config = model::LoadPluginConfiguration<config::TransferConfiguration>(manager.config(), "catapult.plugins.transfer");
		manager.addStatelessValidatorHook([config](auto& builder) {
			builder.add(validators::CreateTransferMessageValidator(config.MaxMessageSize));
			builder.add(validators::CreateTransferMosaicsValidator());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterTransferSubsystem(manager);
}
