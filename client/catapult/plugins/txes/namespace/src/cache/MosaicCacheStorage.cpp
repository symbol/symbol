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

	void MosaicCacheStorage::Save(const ValueType& value, io::OutputStream& output) {
		const auto& history = value.second;
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

	void MosaicCacheStorage::Load(io::InputStream& input, DestinationType& cacheDelta) {
		// - read header
		auto namespaceId = io::Read<NamespaceId>(input);
		auto id = io::Read<MosaicId>(input);
		auto historyDepth = io::Read64(input);

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
