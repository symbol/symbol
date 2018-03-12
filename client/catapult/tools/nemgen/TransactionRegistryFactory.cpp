#include "TransactionRegistryFactory.h"
#include "catapult/plugins/MosaicDefinitionTransactionPlugin.h"
#include "catapult/plugins/MosaicSupplyChangeTransactionPlugin.h"
#include "catapult/plugins/RegisterNamespaceTransactionPlugin.h"
#include "catapult/plugins/TransferTransactionPlugin.h"

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
