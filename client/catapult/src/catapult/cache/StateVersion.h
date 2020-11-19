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
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	/// State version serialization helpers.
	template<typename TSerializerTraits>
	struct StateVersion {
		/// Writes state version to \a output.
		static void Write(io::OutputStream& output) {
			io::Write16(output, TSerializerTraits::State_Version);
		}

		/// Reads state version from \a input and verifies it.
		static void ReadAndCheck(io::InputStream& input) {
			auto version = io::Read16(input);
			if (TSerializerTraits::State_Version != version)
				CATAPULT_THROW_RUNTIME_ERROR_1("serialized state has invalid version", version);
		}
	};
}}
