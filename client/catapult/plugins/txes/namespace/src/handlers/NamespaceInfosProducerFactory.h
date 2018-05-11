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
#include "src/model/NamespaceConstants.h"
#include "catapult/handlers/HandlerTypes.h"
#include "catapult/types.h"

namespace catapult {
	namespace cache { class NamespaceCache; }
	namespace model { struct NamespaceInfo; }
}

namespace catapult { namespace handlers {

	/// Alias for creating a namespace info producer given a range of ids.
	using NamespaceInfosProducerFactory = SharedPointerProducerFactory<NamespaceId, model::NamespaceInfo>;

	/// Creates a namespace infos producer factory around \a namespaceCache.
	NamespaceInfosProducerFactory CreateNamespaceInfosProducerFactory(const cache::NamespaceCache& namespaceCache);
}}
