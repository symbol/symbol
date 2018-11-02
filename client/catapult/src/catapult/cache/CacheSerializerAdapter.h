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
#include "catapult/io/BufferInputStreamAdapter.h"
#include "catapult/io/SizeCalculatingOutputStream.h"
#include "catapult/io/StringOutputStream.h"
#include "catapult/utils/traits/Traits.h"
#include "catapult/types.h"

namespace catapult { namespace cache {

	/// Cache serializer adapter.
	template<typename TSerializerTraits, typename TDescriptor = TSerializerTraits>
	class CacheSerializerAdapter {
	public:
		using KeyType = typename TDescriptor::KeyType;
		using ValueType = typename TDescriptor::ValueType;

	public:
		/// Serializes \a value to string.
		static std::string SerializeValue(const ValueType& value) {
			io::SizeCalculatingOutputStream calculator;
			TSerializerTraits::Save(value, calculator);

			io::StringOutputStream output(calculator.size());
			TSerializerTraits::Save(value, output);
			return output.str();
		}

		/// Deserializes value from \a buffer.
		static ValueType DeserializeValue(const RawBuffer& buffer) {
			io::BufferInputStreamAdapter<RawBuffer> input(buffer);
			return TSerializerTraits::Load(input);
		}
	};
}}
