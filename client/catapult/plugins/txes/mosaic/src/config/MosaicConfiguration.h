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

#pragma once
#include "catapult/model/HeightDependentAddress.h"
#include "catapult/utils/BlockSpan.h"
#include <unordered_set>

namespace catapult {
namespace model {
    struct BlockchainConfiguration;
}
namespace utils {
    class ConfigurationBag;
}
}

namespace catapult {
namespace config {

    /// Mosaic plugin configuration settings.
    struct MosaicConfiguration {
    public:
        /// Maximum number of mosaics that an account can own.
        uint16_t MaxMosaicsPerAccount;

        /// Maximum mosaic duration.
        utils::BlockSpan MaxMosaicDuration;

        /// Maximum mosaic divisibility.
        uint8_t MaxMosaicDivisibility;

        /// Address of the mosaic rental fee sink account (V1).
        Address MosaicRentalFeeSinkAddressV1;

        /// Address of the mosaic rental fee sink account (latest).
        Address MosaicRentalFeeSinkAddress;

        /// Mosaic rental fee.
        Amount MosaicRentalFee;

    private:
        MosaicConfiguration() = default;

    public:
        /// Creates an uninitialized mosaic configuration.
        static MosaicConfiguration Uninitialized();

        /// Loads a mosaic configuration from \a bag.
        static MosaicConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
    };

    /// Gets the mosaic rental fee sink address from \a config and \a blockchainConfig.
    model::HeightDependentAddress GetMosaicRentalFeeSinkAddress(
        const MosaicConfiguration& config,
        const model::BlockchainConfiguration& blockchainConfig);
}
}
