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

#include "MosaicRestrictionEntrySerializer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace state {

	// region Save

	namespace {
		template<typename TSaveKeyPayload>
		void SaveKeyValuePairs(const std::set<uint64_t>& keys, io::OutputStream& output, TSaveKeyPayload saveKeyPayload) {
			io::Write8(output, static_cast<uint8_t>(keys.size()));

			for (auto key : keys) {
				io::Write64(output, key);
				saveKeyPayload(key);
			}
		}

		void SaveRestriction(const MosaicAddressRestriction& restriction, io::OutputStream& output) {
			io::Write(output, restriction.mosaicId());
			output.write(restriction.address());

			SaveKeyValuePairs(restriction.keys(), output, [&restriction, &output](auto key) {
				auto value = restriction.get(key);

				io::Write64(output, value);
			});
		}

		void SaveRestriction(const MosaicGlobalRestriction& restriction, io::OutputStream& output) {
			io::Write(output, restriction.mosaicId());

			SaveKeyValuePairs(restriction.keys(), output, [&restriction, &output](auto key) {
				MosaicGlobalRestriction::RestrictionRule rule;
				restriction.tryGet(key, rule);

				io::Write(output, rule.ReferenceMosaicId);
				io::Write64(output, rule.RestrictionValue);
				io::Write8(output, utils::to_underlying_type(rule.RestrictionType));
			});
		}
	}

	void MosaicRestrictionEntrySerializer::Save(const MosaicRestrictionEntry& entry, io::OutputStream& output) {
		io::Write8(output, utils::to_underlying_type(entry.entryType()));
		if (MosaicRestrictionEntry::EntryType::Address == entry.entryType())
			SaveRestriction(entry.asAddressRestriction(), output);
		else
			SaveRestriction(entry.asGlobalRestriction(), output);
	}

	// endregion

	// region Load

	namespace {
		template<typename TLoadKeyPayload>
		void LoadKeyValuePairs(io::InputStream& input, TLoadKeyPayload loadKeyPayload) {
			auto numKeys = io::Read8(input);

			for (auto i = 0u; i < numKeys; ++i) {
				auto key = io::Read64(input);
				loadKeyPayload(key);
			}
		}

		MosaicAddressRestriction LoadAddressRestriction(io::InputStream& input) {
			auto mosaicId = io::Read<MosaicId>(input);
			Address address;
			input.read(address);

			auto restriction = MosaicAddressRestriction(mosaicId, address);
			LoadKeyValuePairs(input, [&restriction, &input](auto key) {
				auto value = io::Read64(input);

				restriction.set(key, value);
			});

			return restriction;
		}

		MosaicGlobalRestriction LoadGlobalRestriction(io::InputStream& input) {
			auto mosaicId = io::Read<MosaicId>(input);

			auto restriction = MosaicGlobalRestriction(mosaicId);
			LoadKeyValuePairs(input, [&restriction, &input](auto key) {
				MosaicGlobalRestriction::RestrictionRule rule;
				rule.ReferenceMosaicId = io::Read<MosaicId>(input);
				rule.RestrictionValue = io::Read64(input);
				rule.RestrictionType = static_cast<model::MosaicRestrictionType>(io::Read8(input));

				restriction.set(key, rule);
			});

			return restriction;
		}
	}

	MosaicRestrictionEntry MosaicRestrictionEntrySerializer::Load(io::InputStream& input) {
		auto entryType = static_cast<MosaicRestrictionEntry::EntryType>(io::Read8(input));
		switch (entryType) {
		case MosaicRestrictionEntry::EntryType::Address:
			return MosaicRestrictionEntry(LoadAddressRestriction(input));

		case MosaicRestrictionEntry::EntryType::Global:
			return MosaicRestrictionEntry(LoadGlobalRestriction(input));
		}

		CATAPULT_THROW_INVALID_ARGUMENT_1(
				"cannot load mosaic restriction entry with unsupported entry type",
				static_cast<uint16_t>(entryType));
	}

	// endregion
}}
