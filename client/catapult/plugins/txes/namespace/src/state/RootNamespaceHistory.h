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
#include "RootNamespace.h"
#include "catapult/utils/Functional.h"
#include "catapult/plugins.h"
#include <boost/optional.hpp>
#include <list>
#include <set>

namespace catapult { namespace state {

	/// Root namespace history.
	class PLUGIN_API_DEPENDENCY RootNamespaceHistory {
	public:
		static constexpr auto Is_Deactivation_Destructive = true;

	public:
		/// Creates a root namespace history around \a id.
		explicit RootNamespaceHistory(NamespaceId id);

		/// Copy constructor.
		/// \note this constructor is needed to ensure that the copy is sharing children among consecutive roots with same owners.
		RootNamespaceHistory(const RootNamespaceHistory& history);

		/// Move constructor.
		RootNamespaceHistory(RootNamespaceHistory&& history) = default;

	public:
		RootNamespace& operator=(const RootNamespace& rhs) = delete;

	public:
		/// Gets a value indicating whether or not the history is empty.
		bool empty() const;

		/// Gets the id of the root namespace history.
		NamespaceId id() const;

		/// Gets the root namespace history size.
		size_t historyDepth() const;

		/// Gets the number of root namespaces with the same owner starting at the active history.
		size_t activeOwnerHistoryDepth() const;

		/// Gets the number of children of the most recent root namespace.
		size_t numActiveRootChildren() const;

		/// Gets the number of all children.
		/// \note Children are counted more than one time if they are in more than one root namespace.
		size_t numAllHistoricalChildren() const;

	public:
		/// Adds a new root namespace around \a owner and \a lifetime at the end of the history.
		void push_back(const Address& owner, const NamespaceLifetime& lifetime);

		/// Removes the last entry in the history.
		void pop_back();

		/// Gets a const reference to the most recent root namespace.
		const RootNamespace& back() const;

		/// Gets a reference to the most recent root namespace.
		RootNamespace& back();

		/// Prunes all root namespaces that are not active at \a height.
		std::set<NamespaceId> prune(Height height);

	public:
		/// Gets a const iterator to the first root namespace.
		std::list<RootNamespace>::const_iterator begin() const;

		/// Gets a const iterator to the element following the last root namespace.
		std::list<RootNamespace>::const_iterator end() const;

	public:
		/// Returns \c true if history is active at \a height (including grace period).
		/// \note This needs to be called isActive in order for it to dictate state lifetime.
		bool isActive(Height height) const;

	private:
		NamespaceId m_id;
		std::list<RootNamespace> m_rootHistory;
	};
}}
