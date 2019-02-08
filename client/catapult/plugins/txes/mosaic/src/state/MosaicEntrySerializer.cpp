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

#include "MosaicEntrySerializer.h"
#include "catapult/io/PodIoUtils.h"

namespace catapult { namespace state {

	namespace {
		void SaveDefinition(io::OutputStream& output, const MosaicDefinition& definition) {
			io::Write(output, definition.height());
			io::Write(output, definition.owner());
			io::Write32(output, definition.revision());
			for (const auto& property : definition.properties())
				io::Write64(output, property.Value);
		}
	}

	void MosaicEntrySerializer::Save(const MosaicEntry& entry, io::OutputStream& output) {
		if (entry.hasLevy())
			CATAPULT_THROW_RUNTIME_ERROR("cannot save mosaic entry with levy");

		io::Write(output, entry.mosaicId());
		io::Write(output, entry.supply());
		SaveDefinition(output, entry.definition());
	}

	namespace {
		MosaicDefinition LoadDefinition(io::InputStream& input) {
			Key owner;
			auto height = io::Read<Height>(input);
			input.read(owner);
			auto revision = io::Read32(input);

			model::MosaicProperties::PropertyValuesContainer values{};
			for (auto& value : values)
				value = io::Read64(input);

			return MosaicDefinition(height, owner, revision, model::MosaicProperties::FromValues(values));
		}
	}

	MosaicEntry MosaicEntrySerializer::Load(io::InputStream& input) {
		auto mosaicId = io::Read<MosaicId>(input);
		auto supply = io::Read<Amount>(input);
		auto definition = LoadDefinition(input);

		auto entry = MosaicEntry(mosaicId, definition);
		entry.increaseSupply(supply);
		return entry;
	}
}}
