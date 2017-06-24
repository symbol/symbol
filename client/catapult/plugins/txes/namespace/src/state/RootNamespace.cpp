#include "RootNamespace.h"
#include "catapult/state/AccountState.h"

namespace catapult { namespace state {

	Namespace RootNamespace::child(NamespaceId id) const {
		auto iter = m_pChildren->find(id);
		if (m_pChildren->cend() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("unknown child namespace, id= ", id);

		return Namespace(iter->second);
	}

	void RootNamespace::add(const Namespace& ns) {
		// no duplicate children
		if (m_pChildren->cend() != m_pChildren->find(ns.id()))
			CATAPULT_THROW_INVALID_ARGUMENT_1("child namespace already exists, id=", ns.id());

		// parent must be known and must be on same level
		if (id() != ns.parentId()) {
			auto iter = m_pChildren->find(ns.parentId());
			if (m_pChildren->cend() == iter)
				CATAPULT_THROW_INVALID_ARGUMENT_2("child namespace has no parent, (child id, parent id)", ns.id(), ns.parentId());

			if (iter->second.size() != ns.path().size() - 1)
				CATAPULT_THROW_INVALID_ARGUMENT_1("child has incorrect path, id=", ns.id());
		}

		// must have same root
		if (id() != ns.rootId())
			CATAPULT_THROW_INVALID_ARGUMENT_2("child namespace has incorrect root, (child id, root id)", ns.id(), ns.rootId());

		m_pChildren->emplace(ns.id(), ns.path());
	}

	void RootNamespace::remove(NamespaceId id) {
		auto iter = m_pChildren->find(id);
		if (m_pChildren->cend() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot remove child namespace, id=", id);

		// only allow removal if it has no descendents
		// note that all children have a level >= 2
		auto level = iter->second.size();
		auto hasDescendents = std::any_of(m_pChildren->cbegin(), m_pChildren->cend(), [level, id](const auto& pair) {
			const auto& path = pair.second;
			return level < path.size() && id == path[level - 1];
		});

		if (hasDescendents)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot remove child namespace because it has decendents, id=", id);

		m_pChildren->erase(id);
	}

	bool RootNamespace::operator==(const RootNamespace& rhs) const {
		return m_id == rhs.m_id && m_owner == rhs.m_owner;
	}
}}
