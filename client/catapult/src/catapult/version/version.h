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
#include "version_inc.h"
#include <iosfwd>

#define STRINGIFY2(STR) #STR
#define STRINGIFY(STR) STRINGIFY2(STR)

#define CATAPULT_BASE_VERSION \
	STRINGIFY(CATAPULT_VERSION_MAJOR) "." \
	STRINGIFY(CATAPULT_VERSION_MINOR) "." \
	STRINGIFY(CATAPULT_VERSION_REVISION) "." \
	STRINGIFY(CATAPULT_VERSION_BUILD)

#ifdef CATAPULT_VERSION_DESCRIPTION
#define CATAPULT_VERSION CATAPULT_BASE_VERSION " " CATAPULT_VERSION_DESCRIPTION
#else
#define CATAPULT_VERSION CATAPULT_BASE_VERSION
#endif

#define CATAPULT_COPYRIGHT "Copyright (c) Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp."

namespace catapult { namespace version {

	/// Writes custom version information to \a out.
	void WriteVersionInformation(std::ostream& out);
}}
