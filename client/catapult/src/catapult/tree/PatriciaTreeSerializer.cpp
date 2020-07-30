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

#include "PatriciaTreeSerializer.h"
#include "catapult/io/BufferInputStreamAdapter.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/StringOutputStream.h"
#include "catapult/utils/IntegerMath.h"

namespace catapult { namespace tree {

	namespace {
		using LinksMaskType = uint16_t;

		void SerializePath(io::OutputStream& out, const TreeNodePath& path) {
			io::Write8(out, static_cast<uint8_t>(path.size()));

			uint8_t byte = 0;
			for (auto i = 0u; i < path.size(); ++i) {
				if (0 == i % 2) {
					byte = static_cast<uint8_t>(path.nibbleAt(i) << 4);
				} else {
					byte = static_cast<uint8_t>(byte | path.nibbleAt(i));
					io::Write8(out, byte);
				}
			}

			if (1 == path.size() % 2)
				io::Write8(out, byte);
		}

		std::string SerializeLeaf(const LeafTreeNode& node) {
			// marker and path size - uint8, path - hash, value - hash
			io::StringOutputStream out(2 * sizeof(uint8_t) + 2 * Hash256::Size);
			io::Write8(out, 0xFF);
			SerializePath(out, node.path());

			out.write(node.value());
			return out.str();
		}

		std::string SerializeBranch(const BranchTreeNode& node) {
			// marker and path size - uint8, path - hash, at least two links - hashes
			io::StringOutputStream out(2 * sizeof(uint8_t) + 3 * Hash256::Size);
			io::Write8(out, 0x00);
			SerializePath(out, node.path());

			LinksMaskType linksMask = 0;
			for (auto i = 0u; i < utils::GetNumBits<LinksMaskType>(); ++i) {
				auto linksMaskBit = static_cast<LinksMaskType>((node.hasLink(i) ? 1 : 0) << i);
				linksMask = static_cast<LinksMaskType>(linksMask | linksMaskBit);
			}

			io::Write16(out, linksMask);
			for (auto i = 0u; i < utils::GetNumBits<LinksMaskType>(); ++i) {
				if (node.hasLink(i))
					out.write(node.link(i));
			}

			return out.str();
		}

		auto DeserializePath(io::InputStream& input) {
			auto numNibbles = io::Read8(input);
			std::vector<uint8_t> alignedPath((numNibbles + 1) / 2);
			if (alignedPath.empty())
				return TreeNodePath();

			input.read(alignedPath);
			TreeNodePath path(alignedPath);

			// require filler nibble to be 0
			if (1 == numNibbles % 2 && 0 != path.nibbleAt(path.size() - 1))
				CATAPULT_THROW_RUNTIME_ERROR("filler nibble in path must be 0");

			return path.subpath(0, numNibbles);
		}

		auto DeserializeLeaf(io::InputStream& input) {
			auto path = DeserializePath(input);

			Hash256 value;
			input.read(value);
			return LeafTreeNode(path, value);
		}

		auto DeserializeBranch(io::InputStream& input) {
			auto path = DeserializePath(input);
			BranchTreeNode branch(path);

			auto links = io::Read16(input);
			Hash256 link;
			for (auto i = 0u; i < utils::GetNumBits<LinksMaskType>(); ++i) {
				if (links & (1 << i)) {
					input.read(link);
					branch.setLink(link, i);
				}
			}

			return branch;
		}
	}

	std::string PatriciaTreeSerializer::SerializeValue(const TreeNode& value) {
		if (!value.isLeaf() && !value.isBranch())
			CATAPULT_THROW_INVALID_ARGUMENT("invalid node passed to serializer");

		return value.isLeaf()
				? SerializeLeaf(value.asLeafNode())
				: SerializeBranch(value.asBranchNode());
	}

	TreeNode PatriciaTreeSerializer::DeserializeValue(const RawBuffer& buffer) {
		io::BufferInputStreamAdapter<RawBuffer> input(buffer);
		auto nodeMarker = io::Read8(input);
		switch (nodeMarker) {
		case 0xFF:
			return TreeNode(DeserializeLeaf(input));
		case 0x00:
			return TreeNode(DeserializeBranch(input));
		default:
			CATAPULT_THROW_INVALID_ARGUMENT_1("invalid marker of serialized node", nodeMarker);
		}
	}
}}
