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

#include "RootNamespace.h"
#include "catapult/state/AccountState.h"

namespace catapult { namespace state {

	RootNamespace::RootNamespace(NamespaceId id, const Address& ownerAddress, const NamespaceLifetime& lifetime)
			: RootNamespace(id, ownerAddress, lifetime, std::make_shared<Children>())
	{}

	RootNamespace::RootNamespace(
			NamespaceId id,
			const Address& ownerAddress,
			const NamespaceLifetime& lifetime,
			const std::shared_ptr<Children>& pChildren)
			: m_id(id)
			, m_ownerAddress(ownerAddress)
			, m_lifetime(lifetime)
			, m_pChildren(pChildren)
	{}

	NamespaceId RootNamespace::id() const {
		return m_id;
	}

	const RootNamespace::Children& RootNamespace::children() const {
		return *m_pChildren;
	}

	const Address& RootNamespace::ownerAddress() const {
		return m_ownerAddress;
	}

	const NamespaceLifetime& RootNamespace::lifetime() const {
		return m_lifetime;
	}

	bool RootNamespace::empty() const {
		return m_pChildren->empty();
	}

	size_t RootNamespace::size() const {
		return m_pChildren->size();
	}

	Namespace RootNamespace::child(NamespaceId id) const {
		auto iter = m_pChildren->find(id);
		if (m_pChildren->cend() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("unknown child namespace (id) ", id);

		return Namespace(iter->second.Path);
	}

	const NamespaceAlias& RootNamespace::alias(NamespaceId id) const {
		// check if root
		if (m_id == id)
			return m_alias;

		auto iter = m_pChildren->find(id);
		if (m_pChildren->cend() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("unknown child namespace (id) ", id);

		return iter->second.Alias;
	}

	void RootNamespace::add(const Namespace& ns) {
		// no duplicate children
		if (m_pChildren->cend() != m_pChildren->find(ns.id()))
			CATAPULT_THROW_INVALID_ARGUMENT_1("child namespace already exists (id)", ns.id());

		// parent must be known and must be on same level
		if (id() != ns.parentId()) {
			auto iter = m_pChildren->find(ns.parentId());
			if (m_pChildren->cend() == iter)
				CATAPULT_THROW_INVALID_ARGUMENT_2("child namespace has no parent, (child id, parent id)", ns.id(), ns.parentId());

			if (iter->second.Path.size() != ns.path().size() - 1)
				CATAPULT_THROW_INVALID_ARGUMENT_1("child has incorrect path (id)", ns.id());
		}

		// must have same root
		if (id() != ns.rootId())
			CATAPULT_THROW_INVALID_ARGUMENT_2("child namespace has incorrect root, (child id, root id)", ns.id(), ns.rootId());

		m_pChildren->emplace(ns.id(), ns.path());
	}

	void RootNamespace::remove(NamespaceId id) {
		auto iter = m_pChildren->find(id);
		if (m_pChildren->cend() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot remove child namespace (id)", id);

		// only allow removal if it has no descendents
		// note that all children have a level >= 2
		auto level = iter->second.Path.size();
		auto hasDescendents = std::any_of(m_pChildren->cbegin(), m_pChildren->cend(), [level, id](const auto& pair) {
			const auto& path = pair.second.Path;
			return level < path.size() && id == path[level - 1];
		});

		if (hasDescendents)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot remove child namespace because it has decendents (id)", id);

		m_pChildren->erase(id);
	}

	void RootNamespace::setAlias(NamespaceId id, const NamespaceAlias& alias) {
		// check if root
		if (m_id == id){
			m_alias = alias;
			return;
		}

		// child must exist
		auto iter = m_pChildren->find(id);
		if (m_pChildren->cend() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_2("namespace (id) being linked is not under (root)", id, m_id);

		iter->second.Alias = alias;
	}

	bool RootNamespace::operator==(const RootNamespace& rhs) const {
		return m_id == rhs.m_id && m_ownerAddress == rhs.m_ownerAddress;
	}

	bool RootNamespace::operator!=(const RootNamespace& rhs) const {
		return !(*this == rhs);
	}

	RootNamespace RootNamespace::renew(const NamespaceLifetime& newLifetime) const {
		return RootNamespace(m_id, m_ownerAddress, newLifetime, m_pChildren);
	}

	RootNamespace::OrderedChildPaths RootNamespace::sortedChildPaths() const {
		RootNamespace::OrderedChildPaths orderedPaths;
		for (const auto& child : *m_pChildren)
			orderedPaths.insert(child.second.Path);

		return orderedPaths;
	}
}}
