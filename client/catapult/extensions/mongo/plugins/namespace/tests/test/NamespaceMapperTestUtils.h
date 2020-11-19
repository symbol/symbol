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
#include <bsoncxx/json.hpp>

namespace catapult { namespace mongo { namespace plugins { struct NamespaceDescriptor; } } }

namespace catapult { namespace test {

	/// Verifies that the metadata in \a descriptor matches namespace metadata (\a dbMetadata) in db.
	void AssertEqualNamespaceMetadata(const mongo::plugins::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbMetadata);

	/// Verifies that db namespace (\a dbNamespace) and model namespace \a descriptor are equivalent.
	void AssertEqualNamespaceData(const mongo::plugins::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbNamespace);
}}
