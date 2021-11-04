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

#include "NetworkInfo.h"
#include "Address.h"

namespace catapult { namespace model {

	NetworkInfo::NetworkInfo()
			: NetworkInfo(
					NetworkIdentifier::Zero,
					NodeIdentityEqualityStrategy::Key,
					Key(),
					catapult::GenerationHashSeed(),
					utils::TimeSpan())
	{}

	NetworkInfo::NetworkInfo(
			NetworkIdentifier identifier,
			NodeIdentityEqualityStrategy nodeEqualityStrategy,
			const Key& nemesisSignerPublicKey,
			const catapult::GenerationHashSeed& generationHashSeed,
			const utils::TimeSpan& epochAdjustment)
			: Identifier(identifier)
			, NodeEqualityStrategy(nodeEqualityStrategy)
			, NemesisSignerPublicKey(nemesisSignerPublicKey)
			, GenerationHashSeed(generationHashSeed)
			, EpochAdjustment(epochAdjustment)
	{}

	Address GetNemesisSignerAddress(const NetworkInfo& networkInfo) {
		return PublicKeyToAddress(networkInfo.NemesisSignerPublicKey, networkInfo.Identifier);
	}

	UniqueNetworkFingerprint GetUniqueNetworkFingerprint(const NetworkInfo& networkInfo) {
		return UniqueNetworkFingerprint(networkInfo.Identifier, networkInfo.GenerationHashSeed);
	}
}}
