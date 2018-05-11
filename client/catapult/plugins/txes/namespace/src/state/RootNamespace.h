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
#include "catapult/utils/Hashers.h"
#include <unordered_map>

namespace catapult { namespace state {

	/// A root namespace.
	class RootNamespace {
	public:
		using Children = std::unordered_map<NamespaceId, Namespace::Path, utils::BaseValueHasher<NamespaceId>>;

	public:
		/// Creates a root namespace around \a id, \a owner and \a liftime.
		RootNamespace(NamespaceId id, const Key& owner, const NamespaceLifetime& lifetime);

		/// Creates a root namespace around \a id, \a owner, \a liftime and \a pChildren.
		RootNamespace(NamespaceId id, const Key& owner, const NamespaceLifetime& lifetime, const std::shared_ptr<Children>& pChildren);

	public:
		/// Gets the namespace id.
		NamespaceId id() const;

		/// Gets a const reference to the children.
		const Children& children() const;

		/// Gets a const reference to the owner of this namespace.
		const Key& owner() const;

		/// Gets a const reference to the lifetime of this namespace.
		const NamespaceLifetime& lifetime() const;

		/// Returns \c true if this root namespace has no children.
		bool empty() const;

		/// Gets the number of child namespaces.
		size_t size() const;

	public:
		/// Gets a child namespace specified by its namespace \a id.
		/// \note This method throws if the id is unknown.
		Namespace child(NamespaceId id) const;

		/// Adds the child namespace \a ns.
		void add(const Namespace& ns);

		/// Removes a child namespace specified by \a id.
		/// \note This method throws if the id is unknown.
		void remove(NamespaceId id);

	public:
		/// Returns \c true if this root namespace is equal to \a rhs.
		bool operator==(const RootNamespace& rhs) const;

		/// Returns \c true if this root namespace is not equal to \a rhs.
		bool operator!=(const RootNamespace& rhs) const;

	public:
		/// Creates a new root namespace with \a lifetime.
		/// \note The method shares the children of this root namespace with the new root namespace.
		RootNamespace renew(const NamespaceLifetime& newLifetime) const;

	private:
		NamespaceId m_id;
		Key m_owner;
		NamespaceLifetime m_lifetime;
		std::shared_ptr<Children> m_pChildren;
	};
}}
