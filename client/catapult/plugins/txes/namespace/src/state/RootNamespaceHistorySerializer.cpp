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

#include "RootNamespaceHistorySerializer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/utils/Casting.h"
#include <map>
#include <vector>

namespace catapult { namespace state {

	// region RootNamespaceHistoryNonHistoricalSerializer

	namespace {
		enum class HeaderMode { Include_History_Depth, Exclude_History_Depth };

		void SaveHeader(io::OutputStream& output, const RootNamespaceHistory& history, HeaderMode headerMode) {
			if (HeaderMode::Include_History_Depth == headerMode)
				io::Write64(output, history.historyDepth());

			io::Write(output, history.id());
		}

		void SaveAlias(io::OutputStream& output, const NamespaceAlias& alias) {
			io::Write8(output, utils::to_underlying_type(alias.type()));
			switch (alias.type()) {
			case AliasType::Mosaic:
				io::Write(output, alias.mosaicId());
				break;

			case AliasType::Address:
				output.write(alias.address());
				break;

			default:
				break;
			}
		}

		void SaveChildren(io::OutputStream& output, const RootNamespace& root) {
			auto sortedChildPaths = root.sortedChildPaths();
			io::Write64(output, sortedChildPaths.size());
			for (const auto& path : sortedChildPaths) {
				// don't write the first part of the path (the root id) because it is redundant
				io::Write8(output, utils::checked_cast<size_t, uint8_t>(path.size() - 1));

				for (auto i = 1u; i < path.size(); ++i)
					io::Write(output, path[i]);

				SaveAlias(output, root.alias(path[path.size() - 1]));
			}
		}

		const Address& SaveRootNamespace(io::OutputStream& output, const RootNamespace& root, const Address* pLastOwner) {
			output.write(root.ownerAddress());
			io::Write(output, root.lifetime().Start);
			io::Write(output, root.lifetime().End);
			SaveAlias(output, root.alias(root.id()));

			if (pLastOwner && *pLastOwner == root.ownerAddress())
				io::Write64(output, 0); // shared owner, don't rewrite children
			else
				SaveChildren(output, root);

			return root.ownerAddress();
		}
	}

	void RootNamespaceHistoryNonHistoricalSerializer::Save(const RootNamespaceHistory& history, io::OutputStream& output) {
		SaveHeader(output, history, HeaderMode::Exclude_History_Depth);
		if (0 == history.historyDepth())
			CATAPULT_THROW_RUNTIME_ERROR_1("cannot save empty namespace history", history.id());

		SaveRootNamespace(output, history.back(), nullptr);
	}

	namespace {
		struct Header {
			uint64_t HistoryDepth = 0;
			NamespaceId Id;
		};

		Header ReadHeader(io::InputStream& input, HeaderMode headerMode) {
			Header header;
			if (headerMode == HeaderMode::Include_History_Depth)
				header.HistoryDepth = io::Read64(input);

			header.Id = io::Read<NamespaceId>(input);
			return header;
		}

		Namespace::Path LoadPath(io::InputStream& input, NamespaceId rootId) {
			Namespace::Path path;
			path.push_back(rootId);

			auto childDepth = io::Read8(input);
			for (auto i = 0u; i < childDepth; ++i)
				path.push_back(io::Read<NamespaceId>(input));

			return path;
		}

		NamespaceAlias LoadAlias(io::InputStream& input) {
			auto aliasType = AliasType(io::Read8(input));
			switch (aliasType) {
			case AliasType::Mosaic:
				return NamespaceAlias(io::Read<MosaicId>(input));

			case AliasType::Address: {
				Address address;
				input.read(address);
				return NamespaceAlias(address);
			}

			default:
				return NamespaceAlias();
			}
		}

		using ChildDataPairs = std::vector<std::pair<Namespace::Path, NamespaceAlias>>;

		ChildDataPairs LoadChildren(io::InputStream& input, NamespaceId rootId, size_t numChildren) {
			ChildDataPairs childDataPairs;
			for (auto i = 0u; i < numChildren; ++i) {
				auto path = LoadPath(input, rootId);
				auto alias = LoadAlias(input);
				childDataPairs.emplace_back(path, alias);
			}

			return childDataPairs;
		}

		void LoadRootNamespace(io::InputStream& input, RootNamespaceHistory& history) {
			Address owner;
			input.read(owner);
			auto lifetimeStart = io::Read<Height>(input);
			auto lifetimeEnd = io::Read<Height>(input);
			history.push_back(owner, NamespaceLifetime(lifetimeStart, lifetimeEnd));

			auto alias = LoadAlias(input);
			history.back().setAlias(history.id(), alias);

			auto numChildren = io::Read64(input);
			auto childDataPairs = LoadChildren(input, history.id(), numChildren);

			auto& currentRoot = history.back();
			for (const auto& pair : childDataPairs) {
				auto ns = Namespace(pair.first);
				currentRoot.add(ns);
				currentRoot.setAlias(ns.id(), pair.second);
			}
		}
	}

	RootNamespaceHistory RootNamespaceHistoryNonHistoricalSerializer::Load(io::InputStream& input) {
		auto header = ReadHeader(input, HeaderMode::Exclude_History_Depth);
		RootNamespaceHistory history(header.Id);

		LoadRootNamespace(input, history);
		return history;
	}

	// endregion

	// region RootNamespaceHistorySerializer

	void RootNamespaceHistorySerializer::Save(const RootNamespaceHistory& history, io::OutputStream& output) {
		SaveHeader(output, history, HeaderMode::Include_History_Depth);

		const Address* pLastOwner = nullptr;
		for (const auto& root : history)
			pLastOwner = &SaveRootNamespace(output, root, pLastOwner);
	}

	RootNamespaceHistory RootNamespaceHistorySerializer::Load(io::InputStream& input) {
		auto header = ReadHeader(input, HeaderMode::Include_History_Depth);
		RootNamespaceHistory history(header.Id);

		for (auto i = 0u; i < header.HistoryDepth; ++i)
			LoadRootNamespace(input, history);

		return history;
	}

	// endregion
}}
