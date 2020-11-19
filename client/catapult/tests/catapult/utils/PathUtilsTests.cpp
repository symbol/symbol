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

#include "catapult/utils/PathUtils.h"
#include "catapult/types.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS PathUtilsTests

#ifdef _WIN32
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

#define SEPARATOR_2 SEPARATOR SEPARATOR
#define SEPARATOR_3 SEPARATOR SEPARATOR SEPARATOR

	namespace {
		bool IsExpectedSeparator(char ch) {
			switch (ch) {
			case '/':
#ifdef _WIN32
			case '\\':
#endif
				return true;
			}

			return false;
		}
	}

	TEST(TEST_CLASS, IsDirectorySeparatorReturnsTrueOnlyForSlashes) {
		// Act:
		for (int i = std::numeric_limits<char>::min(); i <= std::numeric_limits<char>::max(); ++i) {
			auto ch = static_cast<char>(i);

			// Assert:
			if (IsExpectedSeparator(ch))
				EXPECT_TRUE(IsDirectorySeparator(ch)) << "ch: " << ch;
			else
				EXPECT_FALSE(IsDirectorySeparator(ch)) << "ch: " << ch;
		}
	}

	TEST(TEST_CLASS, AdvanceToEndCanAdvanceToEndOfStrings) {
		// Arrange:
		auto empty = "";
		auto token = "hello";
		auto whitespace = "foo bar";
		auto embeddedNuls = "foo\0bar";

		// Act + Assert:
		EXPECT_EQ(empty + 0, AdvanceToEnd(empty));
		EXPECT_EQ(token + 5, AdvanceToEnd(token));
		EXPECT_EQ(whitespace + 7, AdvanceToEnd(whitespace));
		EXPECT_EQ(embeddedNuls + 3, AdvanceToEnd(embeddedNuls));
	}

	TEST(TEST_CLASS, ExtractFilenameExtractsCorrectPartOfPath) {
		// Assert: edge cases
		EXPECT_STREQ("", ExtractFilename(""));
		EXPECT_STREQ("a", ExtractFilename("a"));
		EXPECT_STREQ("", ExtractFilename(SEPARATOR));
		EXPECT_STREQ("", ExtractFilename("a" SEPARATOR));
		EXPECT_STREQ("", ExtractFilename("a" SEPARATOR_2));
		EXPECT_STREQ("b", ExtractFilename("a" SEPARATOR_2 "b"));
		EXPECT_STREQ("b", ExtractFilename(SEPARATOR_3 "b"));

		// - no directories
		EXPECT_STREQ("foo.cpp", ExtractFilename("foo.cpp"));

		// - one directory
		EXPECT_STREQ("foo.cpp", ExtractFilename("bar" SEPARATOR "foo.cpp"));
		EXPECT_STREQ("foo.cpp", ExtractFilename("bar" SEPARATOR_3 "foo.cpp"));

		// - multiple directories
		EXPECT_STREQ("foo.cpp", ExtractFilename("cat" SEPARATOR "baz" SEPARATOR "bar" SEPARATOR "foo.cpp"));
		EXPECT_STREQ("foo.cpp", ExtractFilename("cat" SEPARATOR_3 "baz" SEPARATOR_3 "bar" SEPARATOR_3 "foo.cpp"));
	}

	TEST(TEST_CLASS, ExtractDirectoryAndFilenameExtractsCorrectPartOfPath) {
		// Assert: edge cases
		EXPECT_STREQ("", ExtractDirectoryAndFilename(""));
		EXPECT_STREQ("a", ExtractDirectoryAndFilename("a"));
		EXPECT_STREQ(SEPARATOR, ExtractDirectoryAndFilename(SEPARATOR));
		EXPECT_STREQ("a" SEPARATOR, ExtractDirectoryAndFilename("a" SEPARATOR));
		EXPECT_STREQ("a" SEPARATOR_2, ExtractDirectoryAndFilename("a" SEPARATOR_2));
		EXPECT_STREQ("a" SEPARATOR_2 "b", ExtractDirectoryAndFilename("a" SEPARATOR_2 "b"));
		EXPECT_STREQ(SEPARATOR_3 "b", ExtractDirectoryAndFilename(SEPARATOR_3 "b"));

		// - no directories
		EXPECT_STREQ("foo.cpp", ExtractDirectoryAndFilename("foo.cpp"));

		// - one directory
		EXPECT_STREQ("bar" SEPARATOR "foo.cpp", ExtractDirectoryAndFilename("bar" SEPARATOR "foo.cpp"));
		EXPECT_STREQ("bar" SEPARATOR_3 "foo.cpp", ExtractDirectoryAndFilename("bar" SEPARATOR_3 "foo.cpp"));

		// - multiple directories
		EXPECT_STREQ("bar" SEPARATOR "foo.cpp", ExtractDirectoryAndFilename("cat" SEPARATOR "baz" SEPARATOR "bar" SEPARATOR "foo.cpp"));
		EXPECT_STREQ(
				"bar" SEPARATOR_3 "foo.cpp",
				ExtractDirectoryAndFilename("cat" SEPARATOR_3 "baz" SEPARATOR_3 "bar" SEPARATOR_3 "foo.cpp"));
	}

	namespace {
		void AssertDirectoryName(const char* expected, const char* input) {
			// Act:
			auto actual = ExtractDirectoryName(input);

			// Assert:
			auto expectedSize = strlen(expected);
			std::stringstream message;
			message << "input: " << input << " E(" << expected << ") != A(" << actual.pData << ", " << actual.Size << ")";
			ASSERT_EQ(expectedSize, actual.Size) << message.str();
			EXPECT_EQ_MEMORY(expected, actual.pData, expectedSize) << message.str();
		}
	}

	TEST(TEST_CLASS, ExtractDirectoryNameExtractsCorrectPartOfPath) {
		// Act + Assert
		// - edge cases
		AssertDirectoryName("", "");
		AssertDirectoryName("", "a");
		AssertDirectoryName("", SEPARATOR);
		AssertDirectoryName("a", "a" SEPARATOR);
		AssertDirectoryName("a", "a" SEPARATOR_2);
		AssertDirectoryName("a", "a" SEPARATOR_2 "b");
		AssertDirectoryName("", SEPARATOR_3 "b");

		// - no directories
		AssertDirectoryName("", "foo.cpp");

		// - one directory
		AssertDirectoryName("bar", "bar" SEPARATOR "foo.cpp");
		AssertDirectoryName("bar", "bar" SEPARATOR_3 "foo.cpp");

		// - multiple directories
		AssertDirectoryName("bar", "cat" SEPARATOR "baz" SEPARATOR "bar" SEPARATOR "foo.cpp");
		AssertDirectoryName("bar", "cat" SEPARATOR_3 "baz" SEPARATOR_3 "bar" SEPARATOR_3 "foo.cpp");
	}

	TEST(TEST_CLASS, CorrectInformationIsExtractedFromRealPaths) {
		// Arrange:
		constexpr auto Nix_Path = "catapult/utils/PathUtils.h";
		constexpr auto Windows_Path = "catapult\\utils\\PathUtils.h";

		// Assert:
		// - both windows and *nix should parse nix paths
		EXPECT_STREQ("PathUtils.h", ExtractFilename(Nix_Path));
		EXPECT_STREQ("utils/PathUtils.h", ExtractDirectoryAndFilename(Nix_Path));
		AssertDirectoryName("utils", Nix_Path);

		// - only windows should parse windows paths
#ifdef _WIN32
		EXPECT_STREQ("PathUtils.h", ExtractFilename(Windows_Path));
		EXPECT_STREQ("utils\\PathUtils.h", ExtractDirectoryAndFilename(Windows_Path));
		AssertDirectoryName("utils", Windows_Path);
#else
		EXPECT_STREQ(Windows_Path, ExtractFilename(Windows_Path));
		EXPECT_STREQ(Windows_Path, ExtractDirectoryAndFilename(Windows_Path));
		AssertDirectoryName("", Windows_Path);
#endif
	}
}}
