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

	void MultisigCacheStorage::Save(const ValueType& value, io::OutputStream& output) {
		const auto& entry = value.second;
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

	void MultisigCacheStorage::Load(io::InputStream& input, DestinationType& cacheDelta) {
		// - read header
		auto minApproval = io::Read8(input);
		auto minRemoval = io::Read8(input);
		Key key;
		input.read(key);

		auto entry = state::MultisigEntry(key);
		entry.setMinApproval(minApproval);
		entry.setMinRemoval(minRemoval);

		LoadKeySet(input, entry.cosignatories());
		LoadKeySet(input, entry.multisigAccounts());

		cacheDelta.insert(entry);
	}
}}
