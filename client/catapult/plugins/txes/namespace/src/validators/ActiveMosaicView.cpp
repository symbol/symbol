#include "Validators.h"
#include "ActiveMosaicView.h"
#include "src/cache/MosaicCache.h"
#include "src/cache/NamespaceCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"

namespace catapult { namespace validators {

	validators::ValidationResult ActiveMosaicView::tryGet(MosaicId id, Height height, const state::MosaicEntry** ppEntry) const {
		// ensure that the mosic is active
		const auto& mosaicCache = m_cache.sub<cache::MosaicCache>();
		if (!mosaicCache.isActive(id, height))
			return Failure_Mosaic_Expired;

		*ppEntry = &mosaicCache.get(id);

		// ensure that the namespace is active
		const auto& namespaceCache = m_cache.sub<cache::NamespaceCache>();
		if (!namespaceCache.isActive((*ppEntry)->namespaceId(), height))
			return Failure_Namespace_Expired;

		return ValidationResult::Success;
	}

	validators::ValidationResult ActiveMosaicView::tryGet(
			MosaicId id,
			Height height,
			const Key& owner,
			const state::MosaicEntry** ppEntry) const {
		auto result = tryGet(id, height, ppEntry);
		if (!IsValidationResultSuccess(result))
			return result;

		return (*ppEntry)->definition().owner() != owner
				? Failure_Mosaic_Owner_Conflict
				: ValidationResult::Success;
	}
}}
