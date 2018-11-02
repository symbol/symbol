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

#include "MosaicHistorySerializer.h"
#include "catapult/io/PodIoUtils.h"

namespace catapult { namespace state {

	// region MosaicHistoryNonHistoricalSerializer

	namespace {
		enum class HeaderMode { Include_History_Depth, Exclude_History_Depth };

		void SaveHeader(io::OutputStream& output, const MosaicHistory& history, HeaderMode headerMode) {
			if (0 == history.historyDepth())
				CATAPULT_THROW_RUNTIME_ERROR_1("cannot save empty mosaic history", history.id());

			if (HeaderMode::Include_History_Depth == headerMode)
				io::Write64(output, history.historyDepth());

			io::Write(output, history.namespaceId());
			io::Write(output, history.id());
		}

		void SaveDefinition(io::OutputStream& output, const MosaicDefinition& definition) {
			io::Write(output, definition.height());
			io::Write(output, definition.owner());
			for (const auto& property : definition.properties())
				io::Write64(output, property.Value);
		}

		void SaveEntry(io::OutputStream& output, const MosaicEntry& mosaicEntry) {
			if (mosaicEntry.hasLevy())
				CATAPULT_THROW_RUNTIME_ERROR("cannot save mosaic entry with levy");

			SaveDefinition(output, mosaicEntry.definition());
			io::Write(output, mosaicEntry.supply());
		}
	}

	void MosaicHistoryNonHistoricalSerializer::Save(const MosaicHistory& history, io::OutputStream& output) {
		SaveHeader(output, history, HeaderMode::Exclude_History_Depth);

		SaveEntry(output, history.back());
	}

	namespace {
		struct Header {
			uint64_t HistoryDepth = 0;
			catapult::NamespaceId NamespaceId;
			catapult::MosaicId MosaicId;
		};

		Header ReadHeader(io::InputStream& input, HeaderMode headerMode) {
			Header header;
			if (headerMode == HeaderMode::Include_History_Depth) {
				header.HistoryDepth = io::Read64(input);

				if (0 == header.HistoryDepth)
					CATAPULT_THROW_RUNTIME_ERROR_1("mosaic history in storage is empty", header.MosaicId);
			}

			header.NamespaceId = io::Read<NamespaceId>(input);
			header.MosaicId = io::Read<MosaicId>(input);
			return header;
		}

		MosaicDefinition LoadDefinition(io::InputStream& input) {
			Key owner;
			auto height = io::Read<Height>(input);
			input.read(owner);

			model::MosaicProperties::PropertyValuesContainer values{};
			for (auto& value : values)
				value = io::Read64(input);

			return MosaicDefinition(height, owner, model::MosaicProperties::FromValues(values));
		}

		void LoadEntry(io::InputStream& input, MosaicHistory& history) {
			auto definition = LoadDefinition(input);
			auto supply = io::Read<Amount>(input);
			history.push_back(definition, supply);
		}
	}

	MosaicHistory MosaicHistoryNonHistoricalSerializer::Load(io::InputStream& input) {
		auto header = ReadHeader(input, HeaderMode::Exclude_History_Depth);
		MosaicHistory history(header.NamespaceId, header.MosaicId);

		LoadEntry(input, history);
		return history;
	}

	// endregion

	// region MosaicHistorySerializer

	void MosaicHistorySerializer::Save(const MosaicHistory& history, io::OutputStream& output) {
		SaveHeader(output, history, HeaderMode::Include_History_Depth);

		for (const auto& mosaicEntry : history)
			SaveEntry(output, mosaicEntry);
	}

	MosaicHistory MosaicHistorySerializer::Load(io::InputStream& input) {
		auto header = ReadHeader(input, HeaderMode::Include_History_Depth);
		MosaicHistory history(header.NamespaceId, header.MosaicId);

		for (auto i = 0u; i < header.HistoryDepth; ++i)
			LoadEntry(input, history);

		return history;
	}

	// endregion
}}
