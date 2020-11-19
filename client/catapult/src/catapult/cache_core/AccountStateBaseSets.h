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
#include "AccountStateCacheSerializers.h"
#include "AccountStateCacheTypes.h"
#include "catapult/cache/CachePatriciaTree.h"
#include "catapult/cache/PatriciaTreeEncoderAdapters.h"
#include "catapult/tree/BasePatriciaTree.h"

namespace catapult { namespace cache {

	using BasicAccountStatePatriciaTree = tree::BasePatriciaTree<
		SerializerHashedKeyEncoder<AccountStatePatriciaTreeSerializer>,
		PatriciaTreeRdbDataSource,
		utils::ArrayHasher<Address>>;

	class AccountStatePatriciaTree : public BasicAccountStatePatriciaTree {
	public:
		using BasicAccountStatePatriciaTree::BasicAccountStatePatriciaTree;
		using Serializer = AccountStatePatriciaTreeSerializer;
	};

	struct AccountStateBaseSetDeltaPointers {
		AccountStateCacheTypes::PrimaryTypes::BaseSetDeltaPointerType pPrimary;
		AccountStateCacheTypes::KeyLookupMapTypes::BaseSetDeltaPointerType pKeyLookupMap;
		std::shared_ptr<AccountStatePatriciaTree::DeltaType> pPatriciaTree;
	};

	struct AccountStateBaseSets : public CacheDatabaseMixin {
	public:
		/// Indicates the set is not ordered.
		using IsOrderedSet = std::false_type;

	public:
		explicit AccountStateBaseSets(const CacheConfiguration& config)
				: CacheDatabaseMixin(config, { "default", "key_lookup" })
				, Primary(GetContainerMode(config), database(), 0)
				, KeyLookupMap(GetContainerMode(config), database(), 1)
				, PatriciaTree(hasPatriciaTreeSupport(), database(), 2)
		{}

	public:
		AccountStateCacheTypes::PrimaryTypes::BaseSetType Primary;
		AccountStateCacheTypes::KeyLookupMapTypes::BaseSetType KeyLookupMap;
		CachePatriciaTree<AccountStatePatriciaTree> PatriciaTree;

	public:
		AccountStateBaseSetDeltaPointers rebase() {
			return { Primary.rebase(), KeyLookupMap.rebase(), PatriciaTree.rebase() };
		}

		AccountStateBaseSetDeltaPointers rebaseDetached() const {
			return { Primary.rebaseDetached(), KeyLookupMap.rebaseDetached(), PatriciaTree.rebaseDetached() };
		}

		void commit() {
			Primary.commit();
			KeyLookupMap.commit();
			PatriciaTree.commit();
			flush();
		}
	};
}}
