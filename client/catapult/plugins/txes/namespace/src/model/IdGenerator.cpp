#include "IdGenerator.h"
#include "NamespaceConstants.h"
#include "catapult/crypto/Hashes.h"

namespace catapult { namespace model {

	namespace {
		template<typename TId>
		TId GenerateId(uint64_t parentId, const RawString& name) noexcept {
			Hash256 result;
			crypto::Sha3_256_Builder sha3;
			sha3.update({
				{ reinterpret_cast<const uint8_t*>(&parentId), sizeof(NamespaceId) },
				{ reinterpret_cast<const uint8_t*>(name.pData), name.Size } });
			sha3.final(result);
			return reinterpret_cast<const TId&>(*result.data());
		}
	}

	NamespaceId GenerateRootNamespaceId(const RawString& name) noexcept {
		return GenerateNamespaceId(Namespace_Base_Id, name);
	}

	NamespaceId GenerateNamespaceId(NamespaceId parentId, const RawString& name) noexcept {
		return GenerateId<NamespaceId>(parentId.unwrap(), name);
	}

	MosaicId GenerateMosaicId(NamespaceId namespaceId, const RawString& name) noexcept {
		return GenerateId<MosaicId>(namespaceId.unwrap(), name);
	}
}}
