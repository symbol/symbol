#include "catapult/plugins/PluginModule.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		constexpr auto Valid_Plugin_Name = "catapult.plugins.transfer";
		constexpr auto Valid_Symbol_Name = "RegisterSubsystem";
	}

	// region module load failure

	namespace {
		void AssertCannotLoadPlugin(const std::string& directory, const std::string& name) {
			// Assert:
			EXPECT_THROW(PluginModule(directory, name), catapult_invalid_argument) << directory << " " << name;
		}
	}

	TEST(PluginModuleTests, CannotLoadPluginFromWrongDirectory) {
		// Assert:
		AssertCannotLoadPlugin("foobar", Valid_Plugin_Name);
	}

	TEST(PluginModuleTests, CannotLoadUnkownPlugin_ExplicitDirectory) {
		// Assert:
		AssertCannotLoadPlugin(test::GetExplicitPluginsDirectory(), "catapult.plugins.awesome");
	}

	TEST(PluginModuleTests, CannotLoadUnkownPlugin_ImplicitDirectory) {
		// Assert:
		AssertCannotLoadPlugin("", "catapult.plugins.awesome");
	}

	// endregion

	// region symbol access failure

	TEST(PluginModuleTests, CannotExtractUnknownSymbol_LoadedModule) {
		// Arrange:
		PluginModule module("", Valid_Plugin_Name);

		// Assert:
		EXPECT_TRUE(module.isLoaded());
		EXPECT_THROW(module.symbol<void*>("abc"), catapult_runtime_error);
	}

	TEST(PluginModuleTests, CannotExtractUnknownSymbol_UnloadedModule) {
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

	TEST(PluginModuleTests, CanExtractKnownSymbol_ExplicitDirectory) {
		// Assert:
		AssertCanLoadPluginAndExtractSymbol(test::GetExplicitPluginsDirectory());
	}

	TEST(PluginModuleTests, CanExtractKnownSymbol_ImplicitDirectory) {
		// Assert:
		AssertCanLoadPluginAndExtractSymbol("");
	}

	// endregion

	// region move / release

	TEST(PluginModuleTests, CanMovePluginModuleWithoutUnloading) {
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

	TEST(PluginModuleTests, CannotExtractKnownSymbolAfterRelease) {
		// Assert:
		AssertCannotExtractKnownSymbolAfterRelease(1);
	}

	TEST(PluginModuleTests, ReleaseIsIdempotent) {
		// Assert:
		AssertCannotExtractKnownSymbolAfterRelease(4);
	}

	// endregion
}}
