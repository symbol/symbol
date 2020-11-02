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
#include "finalization/src/model/FinalizationMessage.h"
#include "catapult/model/FinalizationRound.h"
#include <memory>

namespace catapult {
	namespace crypto { class AggregateBmPrivateKeyTree; }
	namespace finalization { struct FinalizationConfiguration; }
	namespace io {
		class BlockStorageCache;
		class BlockStorageView;
		class ProofStorageCache;
	}
}

namespace catapult { namespace chain {

	/// Describes a prevote.
	struct PrevoteDescriptor {
		/// Prevote round.
		model::FinalizationRound Round;

		/// Block height corresponding to the the first prevote hash.
		catapult::Height Height;

		/// Number of prevote hashes.
		size_t HashesCount;
	};

	/// Consumer that's passed a block storage and a prevote descriptor.
	using PrevoteDescriptorConsumer = consumer<const io::BlockStorageView&, const PrevoteDescriptor&>;

	/// Factory for creating finalization messages.
	class FinalizationMessageFactory {
	public:
		virtual ~FinalizationMessageFactory() = default;

	public:
		/// Creates a prevote message for the specified \a round.
		virtual std::unique_ptr<model::FinalizationMessage> createPrevote(const model::FinalizationRound& round) = 0;

		/// Creates a precommit message for the specified \a round, \a height and \a hash.
		virtual std::unique_ptr<model::FinalizationMessage> createPrecommit(
				const model::FinalizationRound& round,
				Height height,
				const Hash256& hash) = 0;
	};

	/// Creates a factory around \a config, \a blockStorage, \a proofStorage, \a prevoteDescriptorConsumer and \a bmPrivateKeyTree.
	std::unique_ptr<FinalizationMessageFactory> CreateFinalizationMessageFactory(
			const finalization::FinalizationConfiguration& config,
			const io::BlockStorageCache& blockStorage,
			const io::ProofStorageCache& proofStorage,
			const PrevoteDescriptorConsumer& prevoteDescriptorConsumer,
			crypto::AggregateBmPrivateKeyTree&& bmPrivateKeyTree);
}}
