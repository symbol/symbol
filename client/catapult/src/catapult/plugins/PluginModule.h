#pragma once
#include <memory>
#include <string>

namespace catapult { namespace plugins {

	/// A plugin module.
	class PluginModule {
	public:
		/// Creates an unloaded module.
		PluginModule() = default;

		/// Creates a loaded module around a plugin named \a name in \a directory.
		PluginModule(const std::string& directory, const std::string& name);

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
