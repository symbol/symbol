#pragma once
#include "Namespace.h"
#include "RootNamespace.h"

namespace catapult { namespace state {

	/// A pair composed of a namespace and its root.
	class NamespaceEntry {
	public:
		/// Creates an entry around \a ns and \a root.
		explicit NamespaceEntry(const Namespace& ns, const RootNamespace& root)
				: m_ns(ns)
				, m_root(root) {
			if (m_ns.rootId() != m_root.id())
				CATAPULT_THROW_INVALID_ARGUMENT("ns and root parameters are incompatible");
		}

	public:
		/// Gets the namespace.
		const Namespace& ns() const {
			return m_ns;
		}

		/// Gets the root.
		const RootNamespace& root() const {
			return m_root;
		}

	private:
		const Namespace& m_ns;
		const RootNamespace& m_root;
	};
}}
