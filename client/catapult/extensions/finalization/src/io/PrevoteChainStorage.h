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
#include "catapult/model/FinalizationRound.h"
#include "catapult/model/HeightHashPair.h"
#include "catapult/model/RangeTypes.h"

namespace catapult { namespace io { class BlockStorageView; } }

namespace catapult { namespace io {

	/// Describes a prevote chain.
	struct PrevoteChainDescriptor {
		/// Prevote round.
		model::FinalizationRound Round;

		/// Block height corresponding to the the first prevote hash.
		catapult::Height Height;

		/// Number of prevote hashes.
		size_t HashesCount;
	};

	/// Storage for prevote chains.
	class PrevoteChainStorage {
	public:
		virtual ~PrevoteChainStorage() = default;

	public:
		/// Returns \c true if backed up chain for \a round contains \a heightHashPair.
		virtual bool contains(const model::FinalizationRound& round, const model::HeightHashPair& heightHashPair) const = 0;

		/// Loads backed up chain for \a round up to \a maxHeight.
		virtual model::BlockRange load(const model::FinalizationRound& round, Height maxHeight) const = 0;

		/// Backs up prevote chain, specified by \a descriptor, stored in \a blockStorageView.
		virtual void save(const BlockStorageView& blockStorageView, const PrevoteChainDescriptor& descriptor) = 0;

		/// Removes backed up chain for \a round.
		virtual void remove(const model::FinalizationRound& round) = 0;
	};
}}
