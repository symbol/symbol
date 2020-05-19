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
		void SaveProperties(io::OutputStream& output, const model::MosaicProperties& properties) {
			io::Write8(output, utils::to_underlying_type(properties.flags()));
			io::Write8(output, properties.divisibility());
			io::Write(output, properties.duration());
		}

		void SaveDefinition(io::OutputStream& output, const MosaicDefinition& definition) {
			io::Write(output, definition.startHeight());
			output.write(definition.ownerAddress());
			io::Write32(output, definition.revision());

			SaveProperties(output, definition.properties());
		}
	}

	void MosaicEntrySerializer::Save(const MosaicEntry& entry, io::OutputStream& output) {
		io::Write(output, entry.mosaicId());
		io::Write(output, entry.supply());
		SaveDefinition(output, entry.definition());
	}

	namespace {
		model::MosaicProperties LoadProperties(io::InputStream& input) {
			auto flags = static_cast<model::MosaicFlags>(io::Read8(input));
			auto divisibility = io::Read8(input);
			auto duration = io::Read<BlockDuration>(input);
			return model::MosaicProperties(flags, divisibility, duration);
		}

		MosaicDefinition LoadDefinition(io::InputStream& input) {
			Address owner;
			auto height = io::Read<Height>(input);
			input.read(owner);
			auto revision = io::Read32(input);

			auto properties = LoadProperties(input);
			return MosaicDefinition(height, owner, revision, properties);
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
