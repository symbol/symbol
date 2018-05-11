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

#include "MultisigCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace cache {

	namespace {
		void SaveKeySet(io::OutputStream& output, const utils::KeySet& keySet) {
			io::Write64(output, keySet.size());
			for (const auto& key : keySet)
				io::Write(output, key);
		}
	}

	void MultisigCacheStorage::Save(const StorageType& element, io::OutputStream& output) {
		const auto& entry = element.second;
		io::Write8(output, entry.minApproval());
		io::Write8(output, entry.minRemoval());
		io::Write(output, entry.key());

		SaveKeySet(output, entry.cosignatories());
		SaveKeySet(output, entry.multisigAccounts());
	}

	namespace {
		void LoadKeySet(io::InputStream& input, utils::KeySet& keySet) {
			auto numKeys = io::Read64(input);
			while (numKeys--) {
				Key key;
				input.read(key);
				keySet.insert(key);
			}
		}
	}

	state::MultisigEntry MultisigCacheStorage::Load(io::InputStream& input) {
		auto minApproval = io::Read8(input);
		auto minRemoval = io::Read8(input);
		Key key;
		input.read(key);

		auto entry = state::MultisigEntry(key);
		entry.setMinApproval(minApproval);
		entry.setMinRemoval(minRemoval);

		LoadKeySet(input, entry.cosignatories());
		LoadKeySet(input, entry.multisigAccounts());
		return entry;
	}

	void MultisigCacheStorage::LoadInto(io::InputStream& input, DestinationType& cacheDelta) {
		cacheDelta.insert(Load(input));
	}
}}
