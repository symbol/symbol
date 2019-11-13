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

#include "MultisigEntrySerializer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace state {

	namespace {
		void SaveKeySet(io::OutputStream& output, const utils::SortedKeySet& keySet) {
			io::Write64(output, keySet.size());
			for (const auto& key : keySet)
				output.write(key);
		}
	}

	void MultisigEntrySerializer::Save(const MultisigEntry& entry, io::OutputStream& output) {
		io::Write32(output, entry.minApproval());
		io::Write32(output, entry.minRemoval());
		output.write(entry.key());

		SaveKeySet(output, entry.cosignatoryPublicKeys());
		SaveKeySet(output, entry.multisigPublicKeys());
	}

	namespace {
		void LoadKeySet(io::InputStream& input, utils::SortedKeySet& keySet) {
			auto numKeys = io::Read64(input);
			while (numKeys--) {
				Key key;
				input.read(key);
				keySet.insert(key);
			}
		}
	}

	MultisigEntry MultisigEntrySerializer::Load(io::InputStream& input) {
		auto minApproval = io::Read32(input);
		auto minRemoval = io::Read32(input);
		Key key;
		input.read(key);

		auto entry = MultisigEntry(key);
		entry.setMinApproval(minApproval);
		entry.setMinRemoval(minRemoval);

		LoadKeySet(input, entry.cosignatoryPublicKeys());
		LoadKeySet(input, entry.multisigPublicKeys());
		return entry;
	}
}}
