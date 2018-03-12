#pragma once
#include "src/state/MosaicEntry.h"

namespace catapult { namespace test {

	/// Creates mosaic properties with a custom \a duration.
	model::MosaicProperties CreateMosaicPropertiesWithDuration(BlockDuration duration);

	/// Creates a mosaic definition with \a height.
	state::MosaicDefinition CreateMosaicDefinition(Height height);

	/// Creates a mosaic entry with \a id and \a supply.
	state::MosaicEntry CreateMosaicEntry(MosaicId id, Amount supply);

	/// Creates a mosaic entry with \a id, \a height and \a supply.
	state::MosaicEntry CreateMosaicEntry(MosaicId id, Height height, Amount supply);

	/// Creates a mosaic entry around \a namespaceId, \a id, \a height, \a owner, \a supply and \a duration.
	std::shared_ptr<const state::MosaicEntry> CreateMosaicEntry(
			NamespaceId namespaceId,
			MosaicId id,
			Height height,
			const Key& owner,
			Amount supply,
			BlockDuration duration);

	/// Asserts that actual properties (\a actualProperties) exactly match expected properties (\a expectedProperties).
	void AssertMosaicDefinitionProperties(
			const model::MosaicProperties& expectedProperties,
			const model::MosaicProperties& actualProperties);
}}
