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

#include "IdGenerator.h"
#include "plugins/txes/namespace/src/model/NameChecker.h"
#include "plugins/txes/namespace/src/model/NamespaceIdGenerator.h"
#include "catapult/exceptions.h"

namespace catapult { namespace extensions {

	namespace {
		[[noreturn]]
		void ThrowInvalidFqn(const char* reason, const RawString& name) {
			std::ostringstream out;
			out << "fully qualified id is invalid due to " << reason << " (" << name << ")";
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		RawString ExtractPartName(const RawString& name, size_t start, size_t size) {
			if (0 == size)
				ThrowInvalidFqn("empty part", name);

			RawString partName(&name.pData[start], size);
			if (!model::IsValidName(reinterpret_cast<const uint8_t*>(partName.pData), partName.Size))
				ThrowInvalidFqn("invalid part name", name);

			return partName;
		}

		void Append(NamespacePath& path, NamespaceId id, const RawString& name) {
			if (path.capacity() == path.size())
				ThrowInvalidFqn("too many parts", name);

			path.push_back(id);
		}

		template<typename TProcessor>
		size_t Split(const RawString& name, TProcessor processor) {
			auto start = 0u;
			for (auto index = 0u; index < name.Size; ++index) {
				if ('.' != name.pData[index])
					continue;

				processor(start, index - start);
				start = index + 1;
			}

			return start;
		}
	}

	UnresolvedMosaicId GenerateMosaicAliasId(const RawString& name) {
		auto namespacePath = GenerateNamespacePath(name);
		auto namespaceId = namespacePath[namespacePath.size() - 1];
		return UnresolvedMosaicId(namespaceId.unwrap());
	}

	NamespacePath GenerateNamespacePath(const RawString& name) {
		auto namespaceId = Namespace_Base_Id;
		NamespacePath path;
		auto start = Split(name, [&name, &namespaceId, &path](auto substringStart, auto size) {
			namespaceId = model::GenerateNamespaceId(namespaceId, ExtractPartName(name, substringStart, size));
			Append(path, namespaceId, name);
		});

		namespaceId = model::GenerateNamespaceId(namespaceId, ExtractPartName(name, start, name.Size - start));
		Append(path, namespaceId, name);
		return path;
	}
}}
