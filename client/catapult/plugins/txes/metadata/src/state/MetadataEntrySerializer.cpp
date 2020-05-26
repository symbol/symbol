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

#include "MetadataEntrySerializer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/utils/Casting.h"
#include "catapult/exceptions.h"

namespace catapult { namespace state {

	// region Save

	void MetadataEntrySerializer::Save(const MetadataEntry& entry, io::OutputStream& output) {
		const auto& key = entry.key();
		output.write(key.sourceAddress());
		output.write(key.targetAddress());
		io::Write64(output, key.scopedMetadataKey());
		io::Write64(output, key.targetId());
		io::Write8(output, utils::to_underlying_type(key.metadataType()));

		const auto& value = entry.value();
		io::Write16(output, static_cast<uint16_t>(value.size()));
		output.write(value);
	}

	// endregion

	// region Load

	MetadataEntry MetadataEntrySerializer::Load(io::InputStream& input) {
		// 1. read partial key
		model::PartialMetadataKey partialKey;
		input.read(partialKey.SourceAddress);
		input.read(partialKey.TargetAddress);
		partialKey.ScopedMetadataKey = io::Read64(input);

		// 2. read target information and create entry (note that only resolved values are serialized)
		auto targetId = io::Read64(input);
		auto metadataType = static_cast<model::MetadataType>(io::Read8(input));
		MetadataEntry entry(CreateMetadataKey(partialKey, { metadataType, targetId }));

		// 3. read and set value
		auto valueSize = io::Read16(input);
		std::vector<uint8_t> valueBuffer(valueSize);
		input.read(valueBuffer);
		entry.value().update(valueBuffer);

		return entry;
	}

	// endregion
}}
