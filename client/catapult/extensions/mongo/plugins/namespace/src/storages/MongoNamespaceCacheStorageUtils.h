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
#include "catapult/model/NetworkIdentifier.h"
#include <vector>

namespace catapult {
	namespace mongo { namespace plugins { struct NamespaceDescriptor; } }
	namespace state { class RootNamespaceHistory; }
}

namespace catapult { namespace mongo { namespace plugins {

	/// Converts the root namespace \a history into a vector of namespace descriptors.
	std::vector<NamespaceDescriptor> NamespaceDescriptorsFromHistory(const state::RootNamespaceHistory& history);
}}}
