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

#pragma once
#include "NetworkIdentifier.h"
#include "NodeIdentity.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace model {

	/// Information about a network.
	struct NetworkInfo {
	public:
		/// Creates a default, uninitialized network info.
		constexpr NetworkInfo()
				: NetworkInfo(
						NetworkIdentifier::Zero,
						NodeIdentityEqualityStrategy::Key,
						Key(),
						catapult::GenerationHashSeed(),
						utils::TimeSpan())
		{}

		/// Creates a network info around network \a identifier, node equality strategy (\a nodeEqualityStrategy),
		/// nemesis signer public key (\a nemesisSignerPublicKey), nemesis generation hash seed (\a generationHashSeed)
		/// and nemesis epoch time adjustment (\a epochAdjustment).
		constexpr NetworkInfo(
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

	public:
		/// Network identifier.
		NetworkIdentifier Identifier;

		/// Node equality strategy.
		NodeIdentityEqualityStrategy NodeEqualityStrategy;

		/// Nemesis public key.
		Key NemesisSignerPublicKey;

		/// Nemesis generation hash seed.
		catapult::GenerationHashSeed GenerationHashSeed;

		/// Nemesis epoch time adjustment.
		utils::TimeSpan EpochAdjustment;
	};

	/// Gets the unique network fingerprint for \a networkInfo.
	inline UniqueNetworkFingerprint GetUniqueNetworkFingerprint(const NetworkInfo& networkInfo) {
		return UniqueNetworkFingerprint(networkInfo.Identifier, networkInfo.GenerationHashSeed);
	}
}}
