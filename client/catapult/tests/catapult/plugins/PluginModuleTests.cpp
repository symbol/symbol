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

#include "catapult/plugins/PluginModule.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

#define TEST_CLASS PluginModuleTests

	namespace {
		constexpr auto Valid_Plugin_Name = "catapult.plugins.transfer";
		constexpr auto Valid_Symbol_Name = "RegisterSubsystem";

		// region traits

		struct ImplicitScopeTraits {
			using Module = PluginModule;
		};

		template<PluginModule::Scope PluginModuleScope>
		struct ExplicitScopeTraits {
			class Module : public PluginModule {
			public:
				Module() : PluginModule()
				{}

				Module(const std::string& directory, const std::string& name) : PluginModule(directory, name, PluginModuleScope)
				{}
			};
		};

		// endregion
	}

#define SCOPE_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ImplicitScope) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ImplicitScopeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ExplicitLocalScope) { \
			TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExplicitScopeTraits<PluginModule::Scope::Local>>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_ExplicitGlobalScope) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExplicitScopeTraits<PluginModule::Scope::Global>>(); \
	} \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region module load failure

	namespace {
		template<typename TModule>
		void AssertCannotLoadPlugin(const std::string& directory, const std::string& name) {
			// Act + Assert:
			EXPECT_THROW(TModule(directory, name), catapult_invalid_argument) << directory << " " << name;
		}
	}

	SCOPE_BASED_TEST(CannotLoadPluginFromWrongDirectory) {
		AssertCannotLoadPlugin<typename TTraits::Module>("foobar", Valid_Plugin_Name);
	}

	SCOPE_BASED_TEST(CannotLoadUnkownPlugin_ExplicitDirectory) {
		AssertCannotLoadPlugin<typename TTraits::Module>(test::GetExplicitPluginsDirectory(), "catapult.plugins.awesome");
	}

	SCOPE_BASED_TEST(CannotLoadUnkownPlugin_ImplicitDirectory) {
		AssertCannotLoadPlugin<typename TTraits::Module>("", "catapult.plugins.awesome");
	}

	// endregion

	// region symbol access failure

	SCOPE_BASED_TEST(CannotExtractUnknownSymbol_LoadedModule) {
		// Arrange:
		typename TTraits::Module module("", Valid_Plugin_Name);

		// Assert:
		EXPECT_TRUE(module.isLoaded());
		EXPECT_THROW(module.template symbol<void*>("abc"), catapult_runtime_error);
	}

	SCOPE_BASED_TEST(CannotExtractUnknownSymbol_UnloadedModule) {
		// Arrange:
		typename TTraits::Module module;

		// Assert:
		EXPECT_FALSE(module.isLoaded());
		EXPECT_THROW(module.template symbol<void*>("abc"), catapult_runtime_error);
	}

	// endregion

	// region valid plugin + symbol

	namespace {
		template<typename TModule>
		void AssertCanLoadPluginAndExtractSymbol(const std::string& directory) {
			// Arrange:
			TModule module(directory, Valid_Plugin_Name);

			// Act:
			auto pSymbol = module.template symbol<void*>(Valid_Symbol_Name);

			// Assert:
			EXPECT_TRUE(module.isLoaded());
			EXPECT_TRUE(!!pSymbol);
		}
	}

	SCOPE_BASED_TEST(CanExtractKnownSymbol_ExplicitDirectory) {
		AssertCanLoadPluginAndExtractSymbol<typename TTraits::Module>(test::GetExplicitPluginsDirectory());
	}

	SCOPE_BASED_TEST(CanExtractKnownSymbol_ImplicitDirectory) {
		AssertCanLoadPluginAndExtractSymbol<typename TTraits::Module>("");
	}

	// endregion

	// region move / release

	SCOPE_BASED_TEST(CanMovePluginModuleWithoutUnloading) {
		// Arrange:
		typename TTraits::Module module;
		{
			typename TTraits::Module originalModule("", Valid_Plugin_Name);
			module = std::move(originalModule);
		}

		// Act:
		auto pSymbol = module.template symbol<void*>(Valid_Symbol_Name);

		// Assert:
		EXPECT_TRUE(module.isLoaded());
		EXPECT_TRUE(!!pSymbol);
	}

	namespace {
		template<typename TModule>
		void AssertCannotExtractKnownSymbolAfterRelease(size_t numReleases) {
			// Arrange:
			TModule module("", Valid_Plugin_Name);

			// Act:
			for (auto i = 0u; i < numReleases; ++i)
				module.release();

			// Assert:
			EXPECT_FALSE(module.isLoaded());
			EXPECT_THROW(module.template symbol<void*>(Valid_Symbol_Name), catapult_runtime_error);
		}
	}

	SCOPE_BASED_TEST(CannotExtractKnownSymbolAfterRelease) {
		AssertCannotExtractKnownSymbolAfterRelease<typename TTraits::Module>(1);
	}

	SCOPE_BASED_TEST(ReleaseIsIdempotent) {
		AssertCannotExtractKnownSymbolAfterRelease<typename TTraits::Module>(4);
	}

	// endregion
}}
