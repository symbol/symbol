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
