#include "Validators.h"
#include "src/model/IdGenerator.h"
#include "src/model/NameChecker.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace validators {

	using Notification = model::NamespaceNameNotification;
	using NameSet = std::unordered_set<std::string>;

	DECLARE_STATELESS_VALIDATOR(NamespaceName, Notification)(uint8_t maxNameSize, const NameSet& reservedRootNamespaceNames) {
		std::unordered_set<NamespaceId, utils::BaseValueHasher<NamespaceId>> reservedRootIds;
		for (const auto& name : reservedRootNamespaceNames)
			reservedRootIds.emplace(model::GenerateNamespaceId(Namespace_Base_Id, name));

		return MAKE_STATELESS_VALIDATOR(NamespaceName, ([maxNameSize, reservedRootIds](const auto& notification) {
			if (maxNameSize < notification.NameSize || !model::IsValidName(notification.NamePtr, notification.NameSize))
				return Failure_Namespace_Invalid_Name;

			auto name = utils::RawString(reinterpret_cast<const char*>(notification.NamePtr), notification.NameSize);
			if (notification.NamespaceId != model::GenerateNamespaceId(notification.ParentId, name))
				return Failure_Namespace_Name_Id_Mismatch;

			auto namespaceId = Namespace_Base_Id == notification.ParentId ? notification.NamespaceId : notification.ParentId;
			if (reservedRootIds.cend() != reservedRootIds.find(namespaceId))
				return Failure_Namespace_Root_Name_Reserved;

			return ValidationResult::Success;
		}));
	}
}}
