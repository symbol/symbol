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

#pragma once
#include "catapult/deltaset/DeltaElements.h"
#include "catapult/tree/PatriciaTree.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	// region IsActiveAdapter

	namespace detail {
		class IsActiveAdapter {
		public:
			template<typename T>
			static bool IsActive(const T& value, Height height) {
				return IsActive(value, height, IsActiveAccessor<T>());
			}

		private:
			enum class IsActiveType { Unsupported, Supported };
			using UnsupportedIsActiveFlag = std::integral_constant<IsActiveType, IsActiveType::Unsupported>;
			using SupportedIsActiveFlag = std::integral_constant<IsActiveType, IsActiveType::Supported>;

			template<typename T, typename = void>
			struct IsActiveAccessor : UnsupportedIsActiveFlag
			{};

			template<typename T>
			struct IsActiveAccessor<
					T,
					typename utils::traits::enable_if_type<decltype(reinterpret_cast<const T*>(0)->isActive(Height()))>::type>
					: SupportedIsActiveFlag
			{};

		private:
			template<typename T>
			static bool IsActive(const T&, Height, UnsupportedIsActiveFlag) {
				return true;
			}

			template<typename T>
			static bool IsActive(const T& value, Height height, SupportedIsActiveFlag) {
				return value.isActive(height);
			}
		};
	}

	// endregion

	/// Applies all changes in \a set to \a tree for all generations starting at \a minGenerationId through the current generation
	/// given the current chain \a height.
	template<typename TTree, typename TSet>
	void ApplyDeltasToTree(TTree& tree, const TSet& set, uint32_t minGenerationId, Height height) {
		auto needsApplication = [&set, minGenerationId, maxGenerationId = set.generationId()](const auto& key) {
			auto generationId = set.generationId(key);
			return minGenerationId <= generationId && generationId <= maxGenerationId;
		};

		auto deltas = set.deltas();
		for (const auto& pair : deltas.Added) {
			if (needsApplication(pair.first)) {
				// when added, a value is always expected to be active
				if (!detail::IsActiveAdapter::IsActive(pair.second, height))
					CATAPULT_THROW_RUNTIME_ERROR("cannot add inactive value to tree");

				tree.set(pair.first, pair.second);
			}
		}

		for (const auto& pair : deltas.Copied) {
			if (needsApplication(pair.first)) {
				if (detail::IsActiveAdapter::IsActive(pair.second, height))
					tree.set(pair.first, pair.second);
				else
					tree.unset(pair.first);
			}
		}

		for (const auto& pair : deltas.Removed) {
			if (needsApplication(pair.first))
				tree.unset(pair.first);
		}
	}
}}
