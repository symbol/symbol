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
#include "MosaicEntry.h"
#include "src/model/NamespaceConstants.h"
#include "catapult/preprocessor.h"
#include "catapult/types.h"
#include <list>

namespace catapult { namespace state {

	/// A mosaic history.
	class MosaicHistory {
	public:
		/// Creates a mosaic history around \a namespaceId and \a id.
		MosaicHistory(NamespaceId namespaceId, MosaicId id);

	public:
		/// Gets a value indicating whether or not the history is empty.
		bool empty() const;

		/// Gets the namespace id of the mosaic history.
		NamespaceId namespaceId() const;

		/// Gets the id of the mosaic history.
		MosaicId id() const;

		/// Gets the mosaic history size.
		size_t historyDepth() const;

	public:
		/// Adds a new mosaic entry composed of \a definition and \a supply at the end of the history.
		void push_back(const MosaicDefinition& definition, Amount supply);

		/// Removes the last entry in the history.
		void pop_back();

		/// Gets a const reference to the most recent mosaic entry.
		const MosaicEntry& back() const;

		/// Gets a reference to the most recent mosaic entry.
		MosaicEntry& back();

		/// Prunes all mosaic entries that are not active at \a height.
		size_t prune(Height height);

	public:
		/// Returns a const iterator to the first entry.
		std::list<MosaicEntry>::const_iterator begin() const;

		/// Returns a const iterator to the element following the last entry.
		std::list<MosaicEntry>::const_iterator end() const;

	public:
		/// Returns \c true if history is active at \a height.
		bool isActive(Height height) const;

	private:
		NamespaceId m_namespaceId;
		MosaicId m_id;
		std::list<MosaicEntry> m_history;
	};
}}
