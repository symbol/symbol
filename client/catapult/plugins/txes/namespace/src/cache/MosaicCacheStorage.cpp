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

#include "MosaicCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"

namespace catapult { namespace cache {

	namespace {
		void SaveDefinition(io::OutputStream& output, const state::MosaicDefinition& definition) {
			io::Write(output, definition.height());
			io::Write(output, definition.owner());
			for (const auto& property : definition.properties())
				io::Write64(output, property.Value);
		}
	}

	void MosaicCacheStorage::Save(const StorageType& element, io::OutputStream& output) {
		const auto& history = element.second;
		if (0 == history.historyDepth())
			CATAPULT_THROW_RUNTIME_ERROR_1("cannot save empty mosaic history", history.id());

		io::Write(output, history.namespaceId());
		io::Write(output, history.id());
		io::Write64(output, history.historyDepth());

		for (const auto& mosaicEntry : history) {
			if (mosaicEntry.hasLevy())
				CATAPULT_THROW_RUNTIME_ERROR("cannot save mosaic entry with levy");

			SaveDefinition(output, mosaicEntry.definition());
			io::Write(output, mosaicEntry.supply());
		}
	}

	namespace {
		struct Header {
			catapult::NamespaceId NamespaceId;
			catapult::MosaicId MosaicId;
			uint64_t HistoryDepth;
		};

		Header ReadHeader(io::InputStream& input) {
			Header header;
			header.NamespaceId = io::Read<NamespaceId>(input);
			header.MosaicId = io::Read<MosaicId>(input);
			header.HistoryDepth = io::Read64(input);

			if (0 == header.HistoryDepth)
				CATAPULT_THROW_RUNTIME_ERROR_1("mosaic history in storage is empty", header.MosaicId);

			return header;
		}

		state::MosaicDefinition LoadDefinition(io::InputStream& input) {
			Key owner;
			auto height = io::Read<Height>(input);
			input.read(owner);

			model::MosaicProperties::PropertyValuesContainer values{};
			for (auto& value : values)
				value = io::Read64(input);

			return state::MosaicDefinition(height, owner, model::MosaicProperties::FromValues(values));
		}
	}

	state::MosaicHistory MosaicCacheStorage::Load(io::InputStream& input) {
		auto header = ReadHeader(input);
		state::MosaicHistory history(header.NamespaceId, header.MosaicId);

		for (auto i = 0u; i < header.HistoryDepth; ++i) {
			auto definition = LoadDefinition(input);
			auto supply = io::Read<Amount>(input);
			history.push_back(definition, supply);
		}

		return history;
	}

	void MosaicCacheStorage::LoadInto(io::InputStream& input, DestinationType& cacheDelta) {
		auto header = ReadHeader(input);

		for (auto i = 0u; i < header.HistoryDepth; ++i) {
			auto definition = LoadDefinition(input);
			auto supply = io::Read<Amount>(input);
			auto entry = state::MosaicEntry(header.NamespaceId, header.MosaicId, definition);
			entry.increaseSupply(supply);
			cacheDelta.insert(entry);
		}
	}
}}
