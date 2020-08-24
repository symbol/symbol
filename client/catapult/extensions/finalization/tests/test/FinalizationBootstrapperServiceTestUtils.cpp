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

#include "FinalizationBootstrapperServiceTestUtils.h"
#include "finalization/src/FinalizationConfiguration.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "finalization/tests/test/mocks/MockProofStorage.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/nodeps/TestConstants.h"

namespace catapult { namespace test {

	cache::CatapultCache FinalizationBootstrapperServiceTestUtils::CreateCache() {
		return CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
	}

	cache::CatapultCache FinalizationBootstrapperServiceTestUtils::CreateCache(std::vector<AccountKeyPairDescriptor>& keyPairDescriptors) {
		auto config = model::BlockChainConfiguration::Uninitialized();
		config.HarvestingMosaicId = Default_Harvesting_Mosaic_Id;
		config.MinVoterBalance = Amount(2'000'000);

		auto cache = CoreSystemCacheFactory::Create(config);
		auto cacheDelta = cache.createDelta();
		auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
		keyPairDescriptors = AddAccountsWithBalances(accountStateCacheDelta, Height(1), Default_Harvesting_Mosaic_Id, {
			Amount(2'000'000), Amount(4'000'000'000'000), Amount(1'000'000), Amount(6'000'000'000'000)
		});
		cache.commit(Height());
		return cache;
	}

	void FinalizationBootstrapperServiceTestUtils::Register(extensions::ServiceLocator& locator, extensions::ServiceState& state) {
		Register(locator, state, std::make_unique<mocks::MockProofStorage>());
	}

	void FinalizationBootstrapperServiceTestUtils::Register(
			extensions::ServiceLocator& locator,
			extensions::ServiceState& state,
			std::unique_ptr<io::ProofStorage>&& pProofStorage) {
		auto config = finalization::FinalizationConfiguration::Uninitialized();
		config.Size = 3000;
		config.Threshold = 2000;
		config.MaxHashesPerPoint = 64;
		config.OtsKeyDilution = 7;
		config.VotingSetGrouping = 500;

		auto pBootstrapperRegistrar = CreateFinalizationBootstrapperServiceRegistrar(config, std::move(pProofStorage));
		pBootstrapperRegistrar->registerServices(locator, state);
	}
}}
