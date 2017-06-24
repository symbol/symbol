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

	/// Advances \a pStr to its end (the \c NUL terminator).
	CPP14_CONSTEXPR const char* AdvanceToEnd(const char* pStr) {
		const char* pEnd = pStr;
		while (*pEnd != '\0') ++pEnd;
		return pEnd;
	}

	/// Extracts the filename part from \a pFullPath.
	/// e.g. ExtractFilename("cat/bar/baz/foo.cpp") == "foo.cpp"
	CPP14_CONSTEXPR const char* ExtractFilename(const char* pFullPath) {
		const char* pEnd = AdvanceToEnd(pFullPath);
		for (const char* pCh = pEnd - 1; pCh >= pFullPath; --pCh) {
			if (IsDirectorySeparator(*pCh))
				return pCh + 1;
		}

		return pFullPath;
	}

	/// Extracts the last directory and filename from \a pFullPath.
	/// e.g. ExtractDirectoryAndFilename("cat/baz/bar/foo.cpp") == "bar/foo.cpp"
	CPP14_CONSTEXPR const char* ExtractDirectoryAndFilename(const char* pFullPath) {
		const char* pEnd = ExtractFilename(pFullPath);

		// skip consecutive directory separators
		const char* pCh = pEnd - 1;
		for (; pCh >= pFullPath; --pCh) {
			if (!IsDirectorySeparator(*pCh))
				break;
		}

		// find next directory separator
		for (; pCh >= pFullPath; --pCh) {
			if (IsDirectorySeparator(*pCh))
				return pCh + 1;
		}

		return pFullPath;
	}

	/// Extracts the last directory name from \a pFullPath.
	/// e.g. ExtractLastDirectoryName("cat/baz/bar/foo.cpp") == "bar"
	CPP14_CONSTEXPR RawString ExtractDirectoryName(const char* pFullPath) {
		const char* pDirectoryName = ExtractDirectoryAndFilename(pFullPath);
		const char* pEnd = ExtractFilename(pFullPath);

		const char* pCh = pDirectoryName;
		for (; pCh < pEnd; ++pCh) {
			if (IsDirectorySeparator(*pCh))
				break;
		}

		return { pDirectoryName, static_cast<size_t>(pCh - pDirectoryName) };
	}
}}
