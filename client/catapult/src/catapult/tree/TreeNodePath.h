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
#include "catapult/utils/traits/Traits.h"
#include <algorithm>
#include <iosfwd>
#include <vector>
#include <stdint.h>

namespace catapult { namespace tree {

	/// Represents a path in a tree.
	class TreeNodePath {
	public:
		/// Creates a default path.
		TreeNodePath();

		/// Creates a path from \a key.
		template<typename TKey>
		explicit TreeNodePath(TKey key) : m_adjustment(0) {
			if constexpr (utils::traits::is_scalar_v<TKey>) {
				m_size = 2 * sizeof(TKey);
				m_path.resize(sizeof(TKey));

				// copy in big endian byte order
				const auto* pKeyData = reinterpret_cast<const uint8_t*>(&key);
				std::reverse_copy(pKeyData, pKeyData + sizeof(TKey), m_path.begin());
			} else {
				m_size = 2 * key.size();
				m_path.resize(key.size());
				std::copy(key.cbegin(), key.cend(), m_path.begin());
			}
		}

	private:
		TreeNodePath(const std::vector<uint8_t>& path, size_t offset, size_t size);

	public:
		/// Returns \c true if this path is empty.
		bool empty() const;

		/// Gets the number of nibbles in this path.
		size_t size() const;

	public:
		/// Gets the nibble at \a index.
		uint8_t nibbleAt(size_t index) const;

	public:
		/// Returns \c true if this path is equal to \a rhs.
		bool operator==(const TreeNodePath& rhs) const;

		/// Returns \c true if this path is not equal to \a rhs.
		bool operator!=(const TreeNodePath& rhs) const;

	public:
		/// Creates a subpath starting at nibble \a offset.
		TreeNodePath subpath(size_t offset) const;

		/// Creates a subpath starting at nibble \a offset composed of \a size nibbles.
		TreeNodePath subpath(size_t offset, size_t size) const;

	public:
		/// Joins \a lhs and \a rhs into a new path.
		static TreeNodePath Join(const TreeNodePath& lhs, const TreeNodePath& rhs);

		/// Joins \a lhs, \a nibble and \a rhs into a new path.
		static TreeNodePath Join(const TreeNodePath& lhs, uint8_t nibble, const TreeNodePath& rhs);

	private:
		size_t m_size;
		size_t m_adjustment; // used to track odd / even starting nibble
		std::vector<uint8_t> m_path;
	};

	/// Insertion operator for outputting \a path to \a out.
	std::ostream& operator<<(std::ostream& out, const TreeNodePath& path);

	/// Compares two paths (\a lhs and \a rhs) and returns the index of the first non-equal nibble.
	size_t FindFirstDifferenceIndex(const TreeNodePath& lhs, const TreeNodePath& rhs);
}}
