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
#include "NamespaceLifetime.h"
#include "src/model/NamespaceConstants.h"

namespace catapult { namespace state {

	/// Catapult namespace.
	class Namespace {
	public:
		/// Underlying namespace path type.
		using Path = std::vector<NamespaceId>;

	public:
		/// Creates a namespace around \a path.
		explicit Namespace(const Path& path);

	public:
		/// Gets the namespace id.
		NamespaceId id() const;

		/// Gets the parent namespace id.
		NamespaceId parentId() const;

		/// Gets the corresponding root namespace id.
		NamespaceId rootId() const;

		/// Gets a value indicating whether or not this namespace is a root namespace.
		bool isRoot() const;

		/// Gets the path.
		const Path& path() const;

	public:
		/// Creates a child namespace of this namespace with namespace identifier \a id.
		Namespace createChild(NamespaceId id) const;

	public:
		/// Returns \c true if this namespace is equal to \a rhs.
		bool operator==(const Namespace& rhs) const;

		/// Returns \c true if this namespace is not equal to \a rhs.
		bool operator!=(const Namespace& rhs) const;

	private:
		Path m_path;
	};
}}
