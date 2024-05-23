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
#include "catapult/types.h"

namespace catapult { namespace cache {
	class ReadOnlyAccountStateCache;
}}

namespace catapult { namespace cache {

	/// View on top of an account state cache for retrieving importances.
	class ImportanceView {
	public:
		/// Creates a view around \a cache.
		explicit ImportanceView(const ReadOnlyAccountStateCache& cache);

	public:
		/// Tries to populate \a importance with the importance for \a publicKey at \a height.
		bool tryGetAccountImportance(const Key& publicKey, Height height, Importance& importance) const;

		/// Gets the importance for \a publicKey at \a height or a default importance if no importance is set.
		Importance getAccountImportanceOrDefault(const Key& publicKey, Height height) const;

		/// Returns \c true if \a address can harvest at \a height.
		bool canHarvest(const Address& address, Height height) const;

	private:
		const ReadOnlyAccountStateCache& m_cache;
	};
}}
