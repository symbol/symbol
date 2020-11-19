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

#include "PluginModule.h"
#include "catapult/exceptions.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <stdexcept>
#include <dlfcn.h>
#endif

namespace catapult { namespace plugins {

	namespace {
#ifdef _WIN32
		constexpr auto Plugin_Extension = ".dll";
		constexpr auto Name_Prefix = "";
		constexpr auto Directory_Separator = "\\";

		void* CatapultLoad(const std::string& pluginPath, PluginModule::Scope) {
			return ::LoadLibraryA(pluginPath.c_str());
		}

		void CatapultUnload(void* pModule) {
			::FreeLibrary(reinterpret_cast<HMODULE>(pModule));
		}

		void* CatapultGetSymbol(void* pModule, const char* symbolName) {
			return ::GetProcAddress(reinterpret_cast<HMODULE>(pModule), symbolName);
		}
#else
#if defined(__APPLE__)
		constexpr auto Plugin_Extension = ".dylib";
#else
		constexpr auto Plugin_Extension = ".so";
#endif
		constexpr auto Name_Prefix = "lib";
		constexpr auto Directory_Separator = "/";

		void* CatapultLoad(const std::string& pluginPath, PluginModule::Scope scope) {
			return ::dlopen(pluginPath.c_str(), RTLD_NOW | (PluginModule::Scope::Global == scope ? RTLD_GLOBAL : RTLD_LOCAL));
		}

		void CatapultUnload(void* pModule) {
			::dlclose(pModule);
		}

		void* CatapultGetSymbol(void* pModule, const char* symbolName) {
			return ::dlsym(pModule, symbolName);
		}
#endif

		std::string GetPluginPath(const std::string& directory, const std::string& name) {
			return (directory.empty() ? "" : directory + Directory_Separator) + Name_Prefix + name + Plugin_Extension;
		}
	}

	PluginModule::PluginModule(const std::string& directory, const std::string& name) : PluginModule(directory, name, Scope::Local)
	{}

	PluginModule::PluginModule(const std::string& directory, const std::string& name, Scope scope) {
		auto pluginPath = GetPluginPath(directory, name);
		CATAPULT_LOG(info) << "loading plugin from " << pluginPath;

		m_pModule = std::shared_ptr<void>(CatapultLoad(pluginPath, scope), [pluginPath](auto* pModule) {
			if (!pModule)
				return;

			CATAPULT_LOG(debug) << "unloading module " << pModule << " (" << pluginPath << ")";
			CatapultUnload(pModule);
		});

		if (!m_pModule)
			CATAPULT_THROW_INVALID_ARGUMENT_1("unable to find plugin", pluginPath);

		CATAPULT_LOG(debug) << "plugin " << pluginPath << " loaded as " << m_pModule.get();
	}

	void* PluginModule::symbolVoid(const char* symbolName) const {
		if (!m_pModule)
			CATAPULT_THROW_RUNTIME_ERROR("cannot access symbol from unloaded module");

		auto pSymbol = CatapultGetSymbol(m_pModule.get(), symbolName);
		if (!pSymbol)
			CATAPULT_THROW_RUNTIME_ERROR_1("unable to find symbol", std::string(symbolName));

		return pSymbol;
	}

	void PluginModule::release() {
		m_pModule.reset();
	}
}}
