#include "TransactionRegistryFactory.h"
#include "catapult/plugins/MosaicDefinitionTransactionPlugins.h"
#include "catapult/plugins/MosaicSupplyChangeTransactionPlugins.h"
#include "catapult/plugins/RegisterNamespaceTransactionPlugins.h"
#include "catapult/plugins/TransferTransactionPlugins.h"

namespace catapult { namespace tools { namespace nemgen {

	model::TransactionRegistry CreateTransactionRegistry() {
		plugins::MosaicRentalFeeConfiguration mosaicConfig;
		plugins::NamespaceRentalFeeConfiguration namespaceConfig;
		model::TransactionRegistry registry;
		registry.registerPlugin(plugins::CreateMosaicDefinitionTransactionPlugin(mosaicConfig));
		registry.registerPlugin(plugins::CreateMosaicSupplyChangeTransactionPlugin());
		registry.registerPlugin(plugins::CreateRegisterNamespaceTransactionPlugin(namespaceConfig));
		registry.registerPlugin(plugins::CreateTransferTransactionPlugin());
		return registry;
	}
}}}
