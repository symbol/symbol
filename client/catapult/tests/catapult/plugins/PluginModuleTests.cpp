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
		// Assert:
		AssertCannotLoadPlugin("foobar", Valid_Plugin_Name);
	}

	TEST(TEST_CLASS, CannotLoadUnkownPlugin_ExplicitDirectory) {
		// Assert:
		AssertCannotLoadPlugin(test::GetExplicitPluginsDirectory(), "catapult.plugins.awesome");
	}

	TEST(TEST_CLASS, CannotLoadUnkownPlugin_ImplicitDirectory) {
		// Assert:
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
		// Assert:
		AssertCanLoadPluginAndExtractSymbol(test::GetExplicitPluginsDirectory());
	}

	TEST(TEST_CLASS, CanExtractKnownSymbol_ImplicitDirectory) {
		// Assert:
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
		// Assert:
		AssertCannotExtractKnownSymbolAfterRelease(1);
	}

	TEST(TEST_CLASS, ReleaseIsIdempotent) {
		// Assert:
		AssertCannotExtractKnownSymbolAfterRelease(4);
	}

	// endregion
}}
