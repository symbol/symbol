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

#if defined(WIN32) || defined(WIN64)

#ifdef DLL_EXPORTS
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __declspec(dllimport)
#endif

#define PLUGIN_API_EXCEPTION
#define PLUGIN_API_DEPENDENCY

#else
#define PLUGIN_API __attribute__ ((visibility ("default")))

#if defined(__GNUC__) && defined(__clang__)
#define PLUGIN_API_EXCEPTION __attribute__ ((type_visibility ("default")))
#define PLUGIN_API_DEPENDENCY __attribute__ ((type_visibility ("default")))
#else
// GCC doesn't support type_visibility attribute
#define PLUGIN_API_EXCEPTION __attribute__ ((visibility ("default")))
#define PLUGIN_API_DEPENDENCY __attribute__ ((visibility ("default")))
#endif

#endif
