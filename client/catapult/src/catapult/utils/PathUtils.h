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
#include "RawBuffer.h"

namespace catapult { namespace utils {

	/// Gets a value indicating whether or not \a ch is a directory separator.
	constexpr bool IsDirectorySeparator(char ch) {
#ifdef _WIN32
		return '\\' == ch || '/' == ch;
#else
		return '/' == ch;
#endif
	}

	/// Advances \a str to its end (the \c NUL terminator).
	constexpr const char* AdvanceToEnd(const char* str) {
		const auto* pEnd = str;
		while (*pEnd != '\0') ++pEnd;
		return pEnd;
	}

	/// Extracts the filename part from \a fullPath.
	/// e.g. ExtractFilename("cat/bar/baz/foo.cpp") == "foo.cpp"
	constexpr const char* ExtractFilename(const char* fullPath) {
		const auto* pEnd = AdvanceToEnd(fullPath);
		for (const auto* pCh = pEnd - 1; pCh >= fullPath; --pCh) {
			if (IsDirectorySeparator(*pCh))
				return pCh + 1;
		}

		return fullPath;
	}

	/// Extracts the last directory and filename from \a fullPath.
	/// e.g. ExtractDirectoryAndFilename("cat/baz/bar/foo.cpp") == "bar/foo.cpp"
	constexpr const char* ExtractDirectoryAndFilename(const char* fullPath) {
		const auto* pEnd = ExtractFilename(fullPath);

		// skip consecutive directory separators
		const auto* pCh = pEnd - 1;
		for (; pCh >= fullPath; --pCh) {
			if (!IsDirectorySeparator(*pCh))
				break;
		}

		// find next directory separator
		for (; pCh >= fullPath; --pCh) {
			if (IsDirectorySeparator(*pCh))
				return pCh + 1;
		}

		return fullPath;
	}

	/// Extracts the last directory name from \a fullPath.
	/// e.g. ExtractLastDirectoryName("cat/baz/bar/foo.cpp") == "bar"
	constexpr RawString ExtractDirectoryName(const char* fullPath) {
		const auto* pDirectoryName = ExtractDirectoryAndFilename(fullPath);
		const auto* pEnd = ExtractFilename(fullPath);

		const auto* pCh = pDirectoryName;
		for (; pCh < pEnd; ++pCh) {
			if (IsDirectorySeparator(*pCh))
				break;
		}

		return { pDirectoryName, static_cast<size_t>(pCh - pDirectoryName) };
	}
}}
