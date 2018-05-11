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
#include "catapult/ionet/Node.h"
#include <vector>

namespace catapult { namespace config {

	/// Loads peers from the specified stream (\a input) for the network identified by \a networkIdentifier.
	std::vector<ionet::Node> LoadPeersFromStream(std::istream& input, model::NetworkIdentifier networkIdentifier);

	/// Loads peers from the specified \a path for the network identified by \a networkIdentifier.
	std::vector<ionet::Node> LoadPeersFromPath(const std::string& path, model::NetworkIdentifier networkIdentifier);
}}
