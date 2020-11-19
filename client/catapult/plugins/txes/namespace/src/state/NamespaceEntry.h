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
#include "Namespace.h"
#include "RootNamespace.h"
#include "catapult/exceptions.h"

namespace catapult { namespace state {

	/// Pair composed of a namespace and its root.
	class NamespaceEntry {
	public:
		/// Creates an uninitialized entry.
		NamespaceEntry()
				: m_pNamespace(nullptr)
				, m_pRoot(nullptr)
		{}

		/// Creates an entry around \a ns and \a root.
		NamespaceEntry(const Namespace& ns, const RootNamespace& root)
				: m_pNamespace(&ns)
				, m_pRoot(&root) {
			if (m_pNamespace->rootId() != m_pRoot->id())
				CATAPULT_THROW_INVALID_ARGUMENT("ns and root parameters are incompatible");
		}

	public:
		/// Gets the namespace.
		const Namespace& ns() const {
			return *m_pNamespace;
		}

		/// Gets the root.
		const RootNamespace& root() const {
			return *m_pRoot;
		}

	private:
		const Namespace* m_pNamespace;
		const RootNamespace* m_pRoot;
	};
}}
