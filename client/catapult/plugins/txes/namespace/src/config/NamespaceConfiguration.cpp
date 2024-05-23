/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "NamespaceConfiguration.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockchainConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

DEFINE_ADDRESS_CONFIGURATION_VALUE_SUPPORT

namespace catapult {
namespace config {

    NamespaceConfiguration NamespaceConfiguration::Uninitialized()
    {
        return NamespaceConfiguration();
    }

    NamespaceConfiguration NamespaceConfiguration::LoadFromBag(const utils::ConfigurationBag& bag)
    {
        NamespaceConfiguration config;

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "", #NAME, config.NAME)

        LOAD_PROPERTY(MaxNameSize);
        LOAD_PROPERTY(MaxChildNamespaces);
        LOAD_PROPERTY(MaxNamespaceDepth);

        LOAD_PROPERTY(MinNamespaceDuration);
        LOAD_PROPERTY(MaxNamespaceDuration);
        LOAD_PROPERTY(NamespaceGracePeriodDuration);
        LOAD_PROPERTY(ReservedRootNamespaceNames);

        LOAD_PROPERTY(NamespaceRentalFeeSinkAddressV1);
        LOAD_PROPERTY(NamespaceRentalFeeSinkAddress);
        LOAD_PROPERTY(RootNamespaceRentalFeePerBlock);
        LOAD_PROPERTY(ChildNamespaceRentalFee);

#undef LOAD_PROPERTY

        utils::VerifyBagSizeExact(bag, 11);
        return config;
    }

    model::HeightDependentAddress GetNamespaceRentalFeeSinkAddress(
        const NamespaceConfiguration& config,
        const model::BlockchainConfiguration& blockchainConfig)
    {
        model::HeightDependentAddress sinkAddress(config.NamespaceRentalFeeSinkAddress);
        sinkAddress.trySet(config.NamespaceRentalFeeSinkAddressV1, blockchainConfig.ForkHeights.TreasuryReissuance);
        return sinkAddress;
    }
}
}
