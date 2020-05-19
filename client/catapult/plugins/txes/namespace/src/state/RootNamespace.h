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
#include "NamespaceAlias.h"
#include "catapult/utils/Hashers.h"
#include <memory>
#include <set>
#include <unordered_map>

namespace catapult { namespace state {

	/// Root namespace.
	class RootNamespace {
	private:
		struct PathsComparator {
		public:
			bool operator()(const Namespace::Path& lhs, const Namespace::Path& rhs) const {
				return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
			}
		};

	public:
		/// Child namespace information.
		struct ChildNamespaceData {
		public:
			/// Creates data around \a path.
			ChildNamespaceData(const Namespace::Path& path) : Path(path)
			{}

		public:
			/// Child namespace path.
			Namespace::Path Path;

			/// Child namespace alias.
			NamespaceAlias Alias;
		};

		using Children = std::unordered_map<NamespaceId, ChildNamespaceData, utils::BaseValueHasher<NamespaceId>>;
		using OrderedChildPaths = std::set<Namespace::Path, PathsComparator>;

	public:
		/// Creates a root namespace around \a id, \a ownerAddress and \a liftime.
		RootNamespace(NamespaceId id, const Address& ownerAddress, const NamespaceLifetime& lifetime);

		/// Creates a root namespace around \a id, \a ownerAddress, \a liftime and \a pChildren.
		RootNamespace(
				NamespaceId id,
				const Address& ownerAddress,
				const NamespaceLifetime& lifetime,
				const std::shared_ptr<Children>& pChildren);

	public:
		/// Gets the namespace id.
		NamespaceId id() const;

		/// Gets a const reference to the children.
		const Children& children() const;

		/// Gets a const reference to the owner of this namespace.
		const Address& ownerAddress() const;

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

		/// Gets a namespace alias for root or child namespace \a id.
		const NamespaceAlias& alias(NamespaceId id) const;

	public:
		/// Adds the child namespace \a ns.
		void add(const Namespace& ns);

		/// Removes a child namespace specified by \a id.
		/// \note This method throws if the id is unknown.
		void remove(NamespaceId id);

		/// Sets the \a alias associated with root or child namespace \a id.
		void setAlias(NamespaceId id, const NamespaceAlias& alias);

	public:
		/// Returns \c true if this root namespace is equal to \a rhs.
		bool operator==(const RootNamespace& rhs) const;

		/// Returns \c true if this root namespace is not equal to \a rhs.
		bool operator!=(const RootNamespace& rhs) const;

	public:
		/// Creates a new root namespace with \a lifetime.
		/// \note The method shares the children of this root namespace with the new root namespace.
		RootNamespace renew(const NamespaceLifetime& newLifetime) const;

		/// Creates an ordered set of child namespace paths.
		/// \note Child paths are ordered lexicographically.
		OrderedChildPaths sortedChildPaths() const;

	private:
		NamespaceId m_id;
		NamespaceAlias m_alias; // root namespace alias
		Address m_ownerAddress;
		NamespaceLifetime m_lifetime;
		std::shared_ptr<Children> m_pChildren;
	};
}}
