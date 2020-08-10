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
#include "finalization/src/FinalizationConfiguration.h"
#include "finalization/src/model/FinalizationMessage.h"
#include "catapult/crypto_voting/OtsTree.h"
#include <memory>

namespace catapult {
	namespace io {
		class BlockStorageCache;
		class ProofStorageCache;
	}
}

namespace catapult { namespace chain {

	/// Factory for creating finalization messages.
	class FinalizationMessageFactory {
	public:
		/// Creates a factory around \a config, \a blockStorage, \a proofStorage and \a otsTree.
		FinalizationMessageFactory(
				const finalization::FinalizationConfiguration& config,
				const io::BlockStorageCache& blockStorage,
				const io::ProofStorageCache& proofStorage,
				crypto::OtsTree&& otsTree);

	public:
		/// Creates a prevote message.
		std::unique_ptr<model::FinalizationMessage> createPrevote();

		/// Creates a precommit message for the specified \a height and \a hash.
		std::unique_ptr<model::FinalizationMessage> createPrecommit(Height height, const Hash256& hash);

	private:
		finalization::FinalizationConfiguration m_config;
		const io::BlockStorageCache& m_blockStorage;
		const io::ProofStorageCache& m_proofStorage;
		crypto::OtsTree m_otsTree;
	};
}}
