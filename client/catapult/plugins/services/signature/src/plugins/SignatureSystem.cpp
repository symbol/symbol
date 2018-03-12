#include "SignatureSystem.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterSignatureSystem(PluginManager& manager) {
		manager.addStatelessValidatorHook([](auto& builder) {
			builder.add(validators::CreateSignatureValidator());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterSignatureSystem(manager);
}
