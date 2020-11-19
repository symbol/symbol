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
#include <memory>
#include <string>

namespace catapult { namespace plugins {

	/// Plugin module.
	class PluginModule {
	public:
		/// Imported symbol scope.
		enum class Scope {
			/// Symbols are imported locally.
			Local,

			/// Symbols are imported globally.
			Global
		};

	public:
		/// Creates an unloaded module.
		PluginModule() = default;

		/// Creates a loaded module around a plugin named \a name in \a directory.
		PluginModule(const std::string& directory, const std::string& name);

		/// Creates a loaded module around a plugin named \a name in \a directory with specified symbol import \a scope.
		PluginModule(const std::string& directory, const std::string& name, Scope scope);

	public:
		/// Returns \c true if this module wraps a loaded system module.
		bool isLoaded() const {
			return !!m_pModule;
		}

		/// Gets a symbol of the specified type named \a symbolName.
		template<typename TFunc>
		TFunc symbol(const char* symbolName) const {
			return reinterpret_cast<TFunc>(symbolVoid(symbolName));
		}

	private:
		void* symbolVoid(const char* symbolName) const;

	public:
		/// Releases the module.
		void release();

	private:
		std::shared_ptr<void> m_pModule; // the system module
	};
}}
