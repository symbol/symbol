#pragma once
#include "src/model/NamespaceConstants.h"
#include "src/model/NamespaceTypes.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region namespace notification types

/// Defines a namespace notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_NAMESPACE_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Namespace, DESCRIPTION, CODE)

	/// Namespace name was provided.
	DEFINE_NAMESPACE_NOTIFICATION(Name, 0x0011, Validator);

	/// Namespace was registered.
	DEFINE_NAMESPACE_NOTIFICATION(Registration, 0x0012, Validator);

	/// Root namespace was registered.
	DEFINE_NAMESPACE_NOTIFICATION(Root_Registration, 0x0021, All);

	/// Child namespace was registered.
	DEFINE_NAMESPACE_NOTIFICATION(Child_Registration, 0x0022, All);

#undef DEFINE_NAMESPACE_NOTIFICATION

	// endregion

	/// Notification of a namespace name.
	struct NamespaceNameNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Namespace_Name_Notification;

	public:
		/// Creates a notification around \a nameSize and \a pName given \a namespaceId and \a parentId.
		explicit NamespaceNameNotification(
				catapult::NamespaceId namespaceId,
				catapult::NamespaceId parentId,
				uint8_t nameSize,
				const uint8_t* pName)
				: Notification(Notification_Type, sizeof(NamespaceNameNotification))
				, NamespaceId(namespaceId)
				, ParentId(parentId)
				, NameSize(nameSize)
				, NamePtr(pName)
		{}

	public:
		/// The id of the namespace.
		catapult::NamespaceId NamespaceId;

		/// The id of the parent namespace.
		catapult::NamespaceId ParentId;

		/// The size of the name.
		uint8_t NameSize;

		/// Const pointer to the namespace name.
		const uint8_t* NamePtr;
	};

	/// Notification of a namespace registration.
	struct NamespaceNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Namespace_Registration_Notification;

	public:
		/// Creates a notification around \a namespaceType.
		explicit NamespaceNotification(model::NamespaceType namespaceType)
				: Notification(Notification_Type, sizeof(NamespaceNotification))
				, NamespaceType(namespaceType)
		{}

	public:
		/// The type of namespace being registered.
		model::NamespaceType NamespaceType;
	};

	/// Notification of a root namespace registration.
	struct RootNamespaceNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Namespace_Root_Registration_Notification;

	public:
		/// Creates a notification around \a signer, \a namespaceId and \a duration.
		explicit RootNamespaceNotification(const Key& signer, NamespaceId namespaceId, BlockDuration duration)
				: Notification(Notification_Type, sizeof(RootNamespaceNotification))
				, Signer(signer)
				, NamespaceId(namespaceId)
				, Duration(duration)
		{}

	public:
		/// The signer.
		const Key& Signer;

		/// The id of the namespace.
		catapult::NamespaceId NamespaceId;

		/// The number of blocks for which the namespace should be valid.
		BlockDuration Duration;
	};

	/// Notification of a child namespace registration.
	struct ChildNamespaceNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Namespace_Child_Registration_Notification;

	public:
		/// Creates a notification around \a signer, \a namespaceId and \a parentId.
		explicit ChildNamespaceNotification(const Key& signer, NamespaceId namespaceId, NamespaceId parentId)
				: Notification(Notification_Type, sizeof(ChildNamespaceNotification))
				, Signer(signer)
				, NamespaceId(namespaceId)
				, ParentId(parentId)
		{}

	public:
		/// The signer.
		const Key& Signer;

		/// The id of the namespace.
		catapult::NamespaceId NamespaceId;

		/// The id of the parent namespace.
		catapult::NamespaceId ParentId;
	};
}}
