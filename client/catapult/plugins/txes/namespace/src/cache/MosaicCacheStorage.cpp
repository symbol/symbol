#include "MosaicCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"

namespace catapult { namespace cache {

	namespace {
		void SaveDefinition(io::OutputStream& output, const state::MosaicDefinition& definition) {
			io::Write(output, definition.height());
			io::Write(output, definition.owner());
			for (auto iter = definition.properties().cbegin(); definition.properties().cend() != iter; ++iter)
				io::Write(output, iter->Value);
		}
	}

	void MosaicCacheStorage::Save(const ValueType& value, io::OutputStream& output) {
		const auto& history = value.second;
		if (0 == history.historyDepth())
			CATAPULT_THROW_RUNTIME_ERROR_1("cannot save empty mosaic history", history.id());

		io::Write(output, history.namespaceId());
		io::Write(output, history.id());
		io::Write(output, history.historyDepth());

		for (auto iter = history.cbegin(); history.cend() != iter; ++iter) {
			if (iter->hasLevy())
				CATAPULT_THROW_RUNTIME_ERROR("cannot save mosaic entry with levy");

			SaveDefinition(output, iter->definition());
			io::Write(output, iter->supply());
		}
	}

	namespace {
		state::MosaicDefinition LoadDefinition(io::InputStream& input) {
			Key owner;
			auto height = io::Read<Height>(input);
			input.read(owner);

			model::MosaicProperties::PropertyValuesContainer values{};
			for (auto& value : values)
				io::Read(input, value);

			return state::MosaicDefinition(height, owner, model::MosaicProperties::FromValues(values));
		}
	}

	void MosaicCacheStorage::Load(io::InputStream& input, DestinationType& cacheDelta) {
		// - read header
		auto namespaceId = io::Read<NamespaceId>(input);
		auto id = io::Read<MosaicId>(input);
		auto historyDepth = io::Read<uint64_t>(input);

		if (0 == historyDepth)
			CATAPULT_THROW_RUNTIME_ERROR_1("mosaic history in storage is empty", id);

		for (auto i = 0u; i < historyDepth; ++i) {
			auto definition = LoadDefinition(input);
			auto supply = io::Read<Amount>(input);
			auto entry = state::MosaicEntry(namespaceId, id, definition);
			entry.increaseSupply(supply);
			cacheDelta.insert(entry);
		}
	}
}}
