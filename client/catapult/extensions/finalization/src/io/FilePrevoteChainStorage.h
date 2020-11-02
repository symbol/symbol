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
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/FinalizationRound.h"
#include "catapult/model/HeightHashPair.h"

namespace catapult { namespace io {

	/// File prevote chain storage.
	class FilePrevoteChainStorage {
	public:
		/// Creates prevote chain storage around \a dataDirectory.
		explicit FilePrevoteChainStorage(const std::string& dataDirectory);

	public:
		/// Locks \a blockStorage and saves \a numBlocks blocks starting at \a startHeight voted at \a round.
		void saveChain(const BlockStorageCache& blockStorage, const model::FinalizationRound& round, Height startHeight, size_t numBlocks);

		/// Removes blocks voted at \a round.
		void removeChain(const model::FinalizationRound& round);

		/// Loads blocks up to \a maxHeight voted at \a round.
		model::BlockRange loadChain(const model::FinalizationRound& round, Height maxHeight) const;

		/// Returns \c true if \a heightHashPair block is in storage at \a round.
		bool contains(const model::FinalizationRound& round, const model::HeightHashPair& heightHashPair) const;

	private:
		std::string m_dataDirectory;
	};
}}
