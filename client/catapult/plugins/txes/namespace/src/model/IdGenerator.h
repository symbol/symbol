#pragma once
#include "NamespaceConstants.h"

namespace catapult { namespace model {

	/// Generates a root namespace id given \a name.
	NamespaceId GenerateRootNamespaceId(const RawString& name) noexcept;

	/// Generates a namespace id given \a parentId and namespace \a name.
	NamespaceId GenerateNamespaceId(NamespaceId parentId, const RawString& name) noexcept;

	/// Generates a mosaic id given \a namespaceId and \a name.
	MosaicId GenerateMosaicId(NamespaceId namespaceId, const RawString& name) noexcept;
}}
