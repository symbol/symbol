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
#include "CacheDatabaseMixin.h"
#include "CachePatriciaTree.h"
#include "catapult/deltaset/ConditionalContainer.h"
#include <type_traits>

namespace catapult { namespace cache {

	/// Cache types adapter for a cache composed of a single set.
	template<typename TPrimaryTypes, typename IsOrderedFlag = std::false_type>
	struct SingleSetCacheTypesAdapter : public CacheDatabaseMixin {
	public:
		using PrimaryTypes = TPrimaryTypes;

	public:
		/// Wrapper around single delta set.
		struct BaseSetDeltaPointers {
			typename TPrimaryTypes::BaseSetDeltaPointerType pPrimary;
		};

		/// Wrapper around single set.
		struct BaseSets : public CacheDatabaseMixin {
		public:
			/// Indicates the set is ordered (used for capability detection in templates).
			using IsOrderedSet = IsOrderedFlag;

		private:
			static constexpr auto PruningMode = IsOrderedSet() ? FilterPruningMode::Enabled : FilterPruningMode::Disabled;

		public:
			/// Creates base sets around \a config.
			explicit BaseSets(const CacheConfiguration& config)
					: CacheDatabaseMixin(config, { "default" }, PruningMode)
					, Primary(GetContainerMode(config), database(), 0)
			{}

		public:
			typename TPrimaryTypes::BaseSetType Primary;

		public:
			/// Gets a delta based on the same original elements as this set.
			BaseSetDeltaPointers rebase() {
				return { Primary.rebase() };
			}

			/// Gets a delta based on the same original elements as this set
			/// but without the ability to commit any changes to the original set.
			BaseSetDeltaPointers rebaseDetached() const {
				return { Primary.rebaseDetached() };
			}

			/// Commits all changes in the rebased cache.
			/// \a args are forwarded to the commit policy.
			template<typename... TArgs>
			void commit(TArgs&&... args) {
				Primary.commit(std::forward<TArgs>(args)...);
				flush();
			}
		};
	};

	/// Cache types adapter for a cache composed of a single set and a patricia tree.
	template<typename TPrimaryTypes, typename TPatriciaTree>
	struct SingleSetAndPatriciaTreeCacheTypesAdapter : public CacheDatabaseMixin {
	public:
		/// Wrapper around single delta set and patricia tree.
		struct BaseSetDeltaPointers {
			typename TPrimaryTypes::BaseSetDeltaPointerType pPrimary;
			std::shared_ptr<typename TPatriciaTree::DeltaType> pPatriciaTree;
		};

		/// Wrapper around single set and patricia tree.
		template<typename TBaseSetDeltaPointers>
		struct BaseSets : public CacheDatabaseMixin {
		public:
			/// Indicates the set is not ordered.
			using IsOrderedSet = std::false_type;

		public:
			/// Creates base sets around \a config.
			explicit BaseSets(const CacheConfiguration& config)
					: CacheDatabaseMixin(config, { "default" })
					, Primary(GetContainerMode(config), database(), 0)
					, PatriciaTree(hasPatriciaTreeSupport(), database(), 1)
			{}

		public:
			typename TPrimaryTypes::BaseSetType Primary;
			CachePatriciaTree<TPatriciaTree> PatriciaTree;

		public:
			/// Gets a delta based on the same original elements as this set.
			TBaseSetDeltaPointers rebase() {
				TBaseSetDeltaPointers deltaPointers;
				deltaPointers.pPrimary = Primary.rebase();
				deltaPointers.pPatriciaTree = PatriciaTree.rebase();
				return deltaPointers;
			}

			/// Gets a delta based on the same original elements as this set
			/// but without the ability to commit any changes to the original set.
			TBaseSetDeltaPointers rebaseDetached() const {
				TBaseSetDeltaPointers deltaPointers;
				deltaPointers.pPrimary = Primary.rebaseDetached();
				deltaPointers.pPatriciaTree = PatriciaTree.rebaseDetached();
				return deltaPointers;
			}

			/// Commits all changes in the rebased cache.
			void commit() {
				Primary.commit();
				PatriciaTree.commit();
				flush();
			}
		};
	};
}}
