#include "AggregatePlugin.h"
#include "AggregateTransactionPlugin.h"
#include "src/config/AggregateConfiguration.h"
#include "src/model/AggregateEntityType.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterAggregateSubsystem(PluginManager& manager) {
		// configure the aggregate to allow all registered transactions that support embedding
		// (this works because the transaction registry is held by reference)
		const auto& transactionRegistry = manager.transactionRegistry();
		auto config = model::LoadPluginConfiguration<config::AggregateConfiguration>(manager.config(), "catapult.plugins.aggregate");
		manager.addTransactionSupport(CreateAggregateTransactionPlugin(transactionRegistry, model::Entity_Type_Aggregate_Complete));
		if (config.EnableBondedAggregateSupport)
			manager.addTransactionSupport(CreateAggregateTransactionPlugin(transactionRegistry, model::Entity_Type_Aggregate_Bonded));

		manager.addStatelessValidatorHook([config](auto& builder) {
			builder.add(validators::CreateBasicAggregateCosignaturesValidator(
					config.MaxTransactionsPerAggregate,
					config.MaxCosignaturesPerAggregate));
			if (config.EnableStrictCosignatureCheck)
				builder.add(validators::CreateStrictAggregateCosignaturesValidator());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterAggregateSubsystem(manager);
}
