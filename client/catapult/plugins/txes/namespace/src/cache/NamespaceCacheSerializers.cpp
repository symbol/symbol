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

#include "NamespaceCacheSerializers.h"
#include "catapult/io/PodIoUtils.h"

namespace catapult { namespace cache {

	std::string NamespaceFlatMapTypesSerializer::SerializeValue(const ValueType& value) {
		io::StringOutputStream output(sizeof(ValueType));

		StateVersion<NamespaceFlatMapTypesSerializer>::Write(output);
		io::Write64(output, value.path().size());
		for (auto id : value.path())
			io::Write(output, id);

		return output.str();
	}

	state::Namespace NamespaceFlatMapTypesSerializer::DeserializeValue(const RawBuffer& buffer) {
		io::BufferInputStreamAdapter<RawBuffer> input(buffer);
		StateVersion<NamespaceFlatMapTypesSerializer>::ReadAndCheck(input);

		state::Namespace::Path path;
		auto size = io::Read64(input);
		for (auto i = 0u; i < size; ++i)
			path.push_back(io::Read<NamespaceId>(input));

		return state::Namespace(path);
	}
}}
