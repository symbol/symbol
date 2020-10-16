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
#include "catapult/utils/BaseValue.h"
#include <string>
#include <stdint.h>

namespace catapult { namespace ionet {

	struct NodeVersion_tag {};

	/// 32-bit node version where each byte represents a version component { major, minor, revision, build }.
	using NodeVersion = utils::BaseValue<uint32_t, NodeVersion_tag>;

	/// Gets the current server version.
	NodeVersion GetCurrentServerVersion();

	/// Tries to parse \a str into node \a version.
	bool TryParseValue(const std::string& str, NodeVersion& version);
}}
