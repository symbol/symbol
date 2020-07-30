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

#include "TreeNodePath.h"
#include "catapult/utils/HexFormatter.h"
#include <cstring>
#include <ostream>

namespace catapult { namespace tree {

	namespace {
		constexpr size_t CalculateByteSize(size_t offset, size_t size) {
			// (offset, size)
			// (2, 4) [00'11'11'00] => 2 == 4/2
			// (3, 4) [00'01'11'10] => 3 == 4/2 + 1
			// (2, 5) [00'11'11'10] => 3 == 5/2 + 1
			// (3, 5) [00'01'11'11] => 3 == 5/2 + 1
			return size / 2 + ((0 == offset % 2 && 0 == size % 2) ? 0 : 1);
		}
	}

	TreeNodePath::TreeNodePath()
			: m_size(0)
			, m_adjustment(0)
	{}

	TreeNodePath::TreeNodePath(const std::vector<uint8_t>& path, size_t offset, size_t size)
			: m_size(size)
			, m_adjustment(offset % 2) // adjustment is needed to correctly handle paths beginning at odd nibbles
			, m_path(CalculateByteSize(offset, size)) {
		if (0 == m_size)
			return;

		std::memcpy(m_path.data(), &path[offset / 2], m_path.size());
	}

	bool TreeNodePath::empty() const {
		return 0 == m_size;
	}

	size_t TreeNodePath::size() const {
		return m_size;
	}

	uint8_t TreeNodePath::nibbleAt(size_t index) const {
		index += m_adjustment;
		auto byte = m_path[index / 2];

		// return high nibble before low nibble
		return 0 == index % 2 ? ((byte & 0xF0) >> 4) : (byte & 0x0F);
	}

	bool TreeNodePath::operator==(const TreeNodePath& rhs) const {
		return size() == rhs.size() && size() == FindFirstDifferenceIndex(*this, rhs);
	}

	bool TreeNodePath::operator!=(const TreeNodePath& rhs) const {
		return !(*this == rhs);
	}

	TreeNodePath TreeNodePath::subpath(size_t offset) const {
		return subpath(offset, size() - offset);
	}

	TreeNodePath TreeNodePath::subpath(size_t offset, size_t size) const {
		return TreeNodePath(m_path, offset + m_adjustment, size);
	}

	namespace {
		class JoinBuilder {
		public:
			explicit JoinBuilder(size_t size)
					: m_index(0)
					, m_path(size, 0)
			{}

		public:
			const std::vector<uint8_t>& path() {
				return m_path;
			}

		public:
			void addNibble(uint8_t nibble) {
				m_path[m_index / 2] = static_cast<uint8_t>(m_path[m_index / 2] | (0 != m_index % 2 ? (nibble & 0x0F) : (nibble << 4)));
				++m_index;
			}

			void addNibbles(const TreeNodePath& path) {
				for (auto i = 0u; i < path.size(); ++i)
					addNibble(path.nibbleAt(i));
			}

		private:
			size_t m_index;
			std::vector<uint8_t> m_path;
		};
	}

	TreeNodePath TreeNodePath::Join(const TreeNodePath& lhs, const TreeNodePath& rhs) {
		auto joinedPathSize = lhs.size() + rhs.size();
		JoinBuilder builder((joinedPathSize + 1) / 2);
		builder.addNibbles(lhs);
		builder.addNibbles(rhs);
		return TreeNodePath(builder.path(), 0, joinedPathSize);
	}

	TreeNodePath TreeNodePath::Join(const TreeNodePath& lhs, uint8_t nibble, const TreeNodePath& rhs) {
		auto joinedPathSize = lhs.size() + 1 + rhs.size();
		JoinBuilder builder((joinedPathSize + 1) / 2);
		builder.addNibbles(lhs);
		builder.addNibble(nibble);
		builder.addNibbles(rhs);
		return TreeNodePath(builder.path(), 0, joinedPathSize);
	}

	std::ostream& operator<<(std::ostream& out, const TreeNodePath& path) {
		out << "( ";
		for (auto i = 0u; i < path.size(); ++i)
			out << utils::IntegralHexFormatter<uint8_t, 0>(path.nibbleAt(i)) << " ";

		out << ")";
		return out;
	}

	size_t FindFirstDifferenceIndex(const TreeNodePath& lhs, const TreeNodePath& rhs) {
		size_t index = 0;
		for (; index < lhs.size() && index < rhs.size() && lhs.nibbleAt(index) == rhs.nibbleAt(index); ++index);
		return index;
	}
}}
