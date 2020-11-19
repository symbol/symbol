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
#include "catapult/exceptions.h"
#include <memory>
#include <vector>

namespace catapult { namespace model {

	/// Registry of plugins.
	template<typename TPlugin, typename TPluginKey>
	class PluginRegistry {
	public:
		/// Gets the number of registered plugins.
		size_t size() const {
			return m_plugins.size();
		}

		/// Finds the plugin corresponding to \a type or \c nullptr if none is registered.
		const TPlugin* findPlugin(TPluginKey type) const {
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
