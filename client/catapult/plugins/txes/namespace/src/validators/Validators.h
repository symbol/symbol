#pragma once
#include "Results.h"
#include "src/model/MosaicNotifications.h"
#include "src/model/NamespaceNotifications.h"
#include "catapult/model/Notifications.h"
#include "catapult/validators/ValidatorTypes.h"
#include <unordered_set>

namespace catapult { namespace model { struct NamespaceLifetimeConstraints; } }

namespace catapult { namespace validators {

	// region RegisterNamespaceTransaction

	/// A validator implementation that applies to namespace notifications and validates that:
	/// - namespace type is valid
	stateless::NotificationValidatorPointerT<model::NamespaceNotification> CreateNamespaceTypeValidator();

	/// A validator implementation that applies to namespace name notifications and validates that:
	/// - namespace name has a maximum size of \a maxNameSize
	/// - namespace name consists only of allowed characters
	stateless::NotificationValidatorPointerT<model::NamespaceNameNotification> CreateNamespaceNameValidator(uint8_t maxNameSize);

	/// A validator implementation that applies to root namespace notifications and validates that:
	/// - namespace duration is less than or equal to \a maxDuration for root namespace
	/// - namespace duration is zero for child namespace
	/// - name is not in \a reservedRootNamespaceNames
	stateless::NotificationValidatorPointerT<model::RootNamespaceNotification> CreateRootNamespaceValidator(
			ArtifactDuration maxDuration,
			const std::unordered_set<std::string>& reservedRootNamespaceNames);

	/// A validator implementation that applies to root register namespace transactions and validates that:
	/// - the namespace is available and can be created or renewed given namespace lifetime \a constraints
	stateful::NotificationValidatorPointerT<model::RootNamespaceNotification> CreateRootNamespaceAvailabilityValidator(
			const model::NamespaceLifetimeConstraints& constraints);

	/// A validator implementation that applies to child register namespace transactions and validates that:
	/// - the namespace is available and can be created
	stateful::NotificationValidatorPointerT<model::ChildNamespaceNotification> CreateChildNamespaceAvailabilityValidator();

	// endregion

	// region MosaicChangeTransaction

	/// A validator implementation that applies to mosaic change notifications and validates that:
	/// - change transaction owner has permission to change mosaic
	stateful::NotificationValidatorPointerT<model::MosaicChangeNotification> CreateMosaicChangeAllowedValidator();

	// endregion

	// region MosaicDefinitionTransaction

	/// A validator implementation that applies to mosaic name notifications and validates that:
	/// - mosaic name has a maximum size of \a maxNameSize
	/// - mosaic name consists only of allowed characters
	stateless::NotificationValidatorPointerT<model::MosaicNameNotification> CreateMosaicNameValidator(uint8_t maxNameSize);

	/// A validator implementation that applies to mosaic properties notifications and validates that:
	/// - definition has valid mosaic flags
	/// - definition has divisibility no greater than \a maxDivisibility
	/// - mosaic duration has a value not larger than \a maxMosaicDuration
	/// - optional mosaic properties are sorted, known and not duplicative
	stateless::NotificationValidatorPointerT<model::MosaicPropertiesNotification> CreateMosaicPropertiesValidator(
			uint8_t maxDivisibility,
			ArtifactDuration maxMosaicDuration);

	/// A validator implementation that applies to mosaic definition notifications and validates that:
	/// - a mosaic is consistent with its purported namespace
	stateful::NotificationValidatorPointerT<model::MosaicDefinitionNotification> CreateNamespaceMosaicConsistencyValidator();

	/// A validator implementation that applies to mosaic definition notifications and validates that:
	/// - the mosaic is available and can be created or modified
	stateful::NotificationValidatorPointerT<model::MosaicDefinitionNotification> CreateMosaicAvailabilityValidator();

	// endregion

	// region MosaicSupplyChangeTransaction

	/// A validator implementation that applies to mosaic supply change notifications and validates that:
	/// - direction has a valid value
	/// - delta amount is non-zero
	stateless::NotificationValidatorPointerT<model::MosaicSupplyChangeNotification> CreateMosaicSupplyChangeValidator();

	/// A validator implementation that applies to all balance transfer notifications and validates that:
	/// - transferred mosaic is active and is transferable
	stateful::NotificationValidatorPointerT<model::BalanceTransferNotification> CreateMosaicTransferValidator();

	/// A validator implementation that applies to mosaic supply change notifications and validates that:
	/// - the affected mosaic has mutable supply
	/// - decrease does not cause owner amount to become negative
	/// - increase does not cause total divisible units to exceed \a maxDivisibleUnits
	/// \note This validator is dependent on MosaicChangeAllowedValidator.
	stateful::NotificationValidatorPointerT<model::MosaicSupplyChangeNotification> CreateMosaicSupplyChangeAllowedValidator(
			Amount maxDivisibleUnits);

	// endregion
}}
