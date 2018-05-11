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
#include <string>

namespace catapult {
	namespace cache {
		class CatapultCache;
		struct SupplementalData;
	}
}

namespace catapult { namespace filechain {

	/// Save catapult \a cache state along with \a supplementalData into state directory inside \a dataDirectory.
	void SaveState(const std::string& dataDirectory, const cache::CatapultCache& cache, const cache::SupplementalData& supplementalData);

	/// Load catapult \a cache state and \a supplementalData from state directory inside \a dataDirectory.
	/// Returns \c true if data has been loaded, \c false if there was nothing to load.
	bool LoadState(const std::string& dataDirectory, cache::CatapultCache& cache, cache::SupplementalData& supplementalData);
}}
