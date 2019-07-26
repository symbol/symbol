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

#include "catapult/plugins/PluginModule.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

#define TEST_CLASS PluginModuleTests

	namespace {
		constexpr auto Valid_Plugin_Name = "catapult.plugins.transfer";
		constexpr auto Valid_Symbol_Name = "RegisterSubsystem";
	}

	// region module load failure

	namespace {
		void AssertCannotLoadPlugin(const std::string& directory, const std::string& name) {
			// Act + Assert:
			EXPECT_THROW(PluginModule(directory, name), catapult_invalid_argument) << directory << " " << name;
		}
	}

	TEST(TEST_CLASS, CannotLoadPluginFromWrongDirectory) {
		AssertCannotLoadPlugin("foobar", Valid_Plugin_Name);
	}

	TEST(TEST_CLASS, CannotLoadUnkownPlugin_ExplicitDirectory) {
		AssertCannotLoadPlugin(test::GetExplicitPluginsDirectory(), "catapult.plugins.awesome");
	}

	TEST(TEST_CLASS, CannotLoadUnkownPlugin_ImplicitDirectory) {
		AssertCannotLoadPlugin("", "catapult.plugins.awesome");
	}

	// endregion

	// region symbol access failure

	TEST(TEST_CLASS, CannotExtractUnknownSymbol_LoadedModule) {
		// Arrange:
		PluginModule module("", Valid_Plugin_Name);

		// Assert:
		EXPECT_TRUE(module.isLoaded());
		EXPECT_THROW(module.symbol<void*>("abc"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotExtractUnknownSymbol_UnloadedModule) {
		// Arrange:
		PluginModule module;

		// Assert:
		EXPECT_FALSE(module.isLoaded());
		EXPECT_THROW(module.symbol<void*>("abc"), catapult_runtime_error);
	}

	// endregion

	// region valid plugin + symbol

	namespace {
		void AssertCanLoadPluginAndExtractSymbol(const std::string& directory) {
			// Arrange:
			PluginModule module(directory, Valid_Plugin_Name);

			// Act:
			auto pSymbol = module.symbol<void*>(Valid_Symbol_Name);

			// Assert:
			EXPECT_TRUE(module.isLoaded());
			EXPECT_TRUE(!!pSymbol);
		}
	}

	TEST(TEST_CLASS, CanExtractKnownSymbol_ExplicitDirectory) {
		AssertCanLoadPluginAndExtractSymbol(test::GetExplicitPluginsDirectory());
	}

	TEST(TEST_CLASS, CanExtractKnownSymbol_ImplicitDirectory) {
		AssertCanLoadPluginAndExtractSymbol("");
	}

	// endregion

	// region move / release

	TEST(TEST_CLASS, CanMovePluginModuleWithoutUnloading) {
		// Arrange:
		PluginModule module;
		{
			PluginModule originalModule("", Valid_Plugin_Name);
			module = std::move(originalModule);
		}

		// Act:
		auto pSymbol = module.symbol<void*>(Valid_Symbol_Name);

		// Assert:
		EXPECT_TRUE(module.isLoaded());
		EXPECT_TRUE(!!pSymbol);
	}

	namespace {
		void AssertCannotExtractKnownSymbolAfterRelease(size_t numReleases) {
			// Arrange:
			PluginModule module("", Valid_Plugin_Name);

			// Act:
			for (auto i = 0u; i < numReleases; ++i)
				module.release();

			// Assert:
			EXPECT_FALSE(module.isLoaded());
			EXPECT_THROW(module.symbol<void*>(Valid_Symbol_Name), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, CannotExtractKnownSymbolAfterRelease) {
		AssertCannotExtractKnownSymbolAfterRelease(1);
	}

	TEST(TEST_CLASS, ReleaseIsIdempotent) {
		AssertCannotExtractKnownSymbolAfterRelease(4);
	}

	// endregion
}}
