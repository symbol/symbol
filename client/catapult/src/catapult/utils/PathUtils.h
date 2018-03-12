#pragma once
#include "RawBuffer.h"
#include "catapult/preprocessor.h"

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
	CPP14_CONSTEXPR const char* AdvanceToEnd(const char* str) {
		const auto* pEnd = str;
		while (*pEnd != '\0') ++pEnd;
		return pEnd;
	}

	/// Extracts the filename part from \a fullPath.
	/// e.g. ExtractFilename("cat/bar/baz/foo.cpp") == "foo.cpp"
	CPP14_CONSTEXPR const char* ExtractFilename(const char* fullPath) {
		const auto* pEnd = AdvanceToEnd(fullPath);
		for (const auto* pCh = pEnd - 1; pCh >= fullPath; --pCh) {
			if (IsDirectorySeparator(*pCh))
				return pCh + 1;
		}

		return fullPath;
	}

	/// Extracts the last directory and filename from \a fullPath.
	/// e.g. ExtractDirectoryAndFilename("cat/baz/bar/foo.cpp") == "bar/foo.cpp"
	CPP14_CONSTEXPR const char* ExtractDirectoryAndFilename(const char* fullPath) {
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
	CPP14_CONSTEXPR RawString ExtractDirectoryName(const char* fullPath) {
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
