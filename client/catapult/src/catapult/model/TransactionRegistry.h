#pragma once
#include "catapult/model/EntityType.h"
#include "catapult/exceptions.h"
#include <memory>
#include <vector>

namespace catapult { namespace model {

	/// A registry of transaction plugins.
	template<typename TPlugin>
	class TransactionRegistryT {
	public:
		/// Gets the number of registered plugins.
		size_t size() const {
			return m_plugins.size();
		}

		/// Finds the plugin corresponding to \a type or \c nullptr if none is registered.
		const TPlugin* findPlugin(EntityType type) const {
			auto iter = std::find_if(m_plugins.cbegin(), m_plugins.cend(), [type](const auto& pPlugin) {
				return pPlugin->type() == type;
			});

			return m_plugins.cend() == iter ? nullptr : iter->get();
		}

	public:
		/// Registers \a pPlugin with the registry.
		void registerPlugin(std::unique_ptr<const TPlugin>&& pPlugin) {
			if (findPlugin(pPlugin->type()))
				CATAPULT_THROW_INVALID_ARGUMENT_1("plugin has already been registered with type", pPlugin->type());

			m_plugins.push_back(std::move(pPlugin));
		}

	private:
		std::vector<std::unique_ptr<const TPlugin>> m_plugins;
	};
}}
