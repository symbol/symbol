#include "MosaicDefinition.h"
#include "src/model/NamespaceConstants.h"

namespace catapult { namespace state {

	bool MosaicDefinition::isEternal() const {
		return Eternal_Artifact_Duration == m_properties.duration();
	}

	bool MosaicDefinition::isActive(Height height) const {
		return isEternal() || (height < Height(m_height.unwrap() + m_properties.duration().unwrap()) && height >= m_height);
	}

	bool MosaicDefinition::isExpired(Height height) const {
		return !isEternal() && m_height + Height(m_properties.duration().unwrap()) <= height;
	}
}}
