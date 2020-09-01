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
#include <memory>

namespace catapult {
	namespace crypto { class OtsTree; }
	namespace finalization { struct FinalizationConfiguration; }
	namespace io {
		class BlockStorageCache;
		class ProofStorageCache;
	}
}

namespace catapult { namespace chain {

	/// Factory for creating finalization messages.
	class FinalizationMessageFactory {
	public:
		virtual ~FinalizationMessageFactory() = default;

	public:
		/// Creates a prevote message for the specified \a point.
		virtual std::unique_ptr<model::FinalizationMessage> createPrevote(FinalizationPoint point) = 0;

		/// Creates a precommit message for the specified \a point, \a height and \a hash.
		virtual std::unique_ptr<model::FinalizationMessage> createPrecommit(
				FinalizationPoint point,
				Height height,
				const Hash256& hash) = 0;
	};

	/// Creates a factory around \a config, \a blockStorage, \a proofStorage and \a otsTree.
	std::unique_ptr<FinalizationMessageFactory> CreateFinalizationMessageFactory(
			const finalization::FinalizationConfiguration& config,
			const io::BlockStorageCache& blockStorage,
			const io::ProofStorageCache& proofStorage,
			crypto::OtsTree&& otsTree);
}}
