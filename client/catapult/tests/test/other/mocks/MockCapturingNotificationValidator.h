#pragma once
#include "MockNotificationValidator.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"

namespace catapult { namespace mocks {

	/// Information about a captured transaction notification.
	struct CapturedTransactionNotificationInfo {
	public:
		/// Creates a captured transaction notification info around \a notification.
		explicit CapturedTransactionNotificationInfo(const model::Notification& notification) {
			IsSet = model::Core_Transaction_Notification == notification.Type;
			if (IsSet) {
				const auto& transactionNotification = static_cast<const model::TransactionNotification&>(notification);
				TransactionHash = transactionNotification.TransactionHash;
				Deadline = transactionNotification.Deadline;
			}
		}

	public:
		/// \c true if a transaction notification was captured.
		bool IsSet;

		/// The transaction hash.
		Hash256 TransactionHash;

		/// The transaction deadline.
		Timestamp Deadline;
	};

	/// Stateless notification validator params.
	struct StatelessNotificationValidatorParams {
	public:
		/// Creates params around \a notification.
		explicit StatelessNotificationValidatorParams(const model::Notification& notification)
				: Notification(notification)
				, TransactionNotificationInfo(notification)
		{}

	public:
		/// The notification.
		const model::Notification& Notification;

		/// The transaction notification information (if applicable).
		CapturedTransactionNotificationInfo TransactionNotificationInfo;
	};

	/// A mock stateless notification validator that captures parameters passed to validate.
	class MockCapturingStatelessNotificationValidator
			: public mocks::MockStatelessNotificationValidatorT<model::Notification>
			, public test::ParamsCapture<StatelessNotificationValidatorParams> {
	private:
		using BaseType = mocks::MockStatelessNotificationValidatorT<model::Notification>;

	public:
		using BaseType::MockStatelessNotificationValidatorT;

	public:
		validators::ValidationResult validate(const model::Notification& notification) const override {
			const_cast<MockCapturingStatelessNotificationValidator*>(this)->push(notification);
			return BaseType::validate(notification);
		}
	};

	/// Stateful notification validator params.
	struct StatefulNotificationValidatorParams {
	public:
		/// Creates params around \a notification and \a context.
		explicit StatefulNotificationValidatorParams(const model::Notification& notification, const validators::ValidatorContext& context)
				: Notification(notification)
				, TransactionNotificationInfo(notification)
				, Height(context.Height)
				, BlockTime(context.BlockTime)
				, NetworkIdentifier(context.Network.Identifier)
				, IsMarkedCache(test::IsMarkedCache(context.Cache))
		{}

	public:
		/// The notification.
		const model::Notification& Notification;

		/// The transaction notification information (if applicable).
		CapturedTransactionNotificationInfo TransactionNotificationInfo;

		/// The validation height.
		const catapult::Height Height;

		/// The validation block time.
		const Timestamp BlockTime;

		/// The validation network.
		const model::NetworkIdentifier NetworkIdentifier;

		/// \c true if the validation cache is marked.
		bool IsMarkedCache;
	};

	/// A mock stateful notification validator that captures parameters passed to validate.
	class MockCapturingStatefulNotificationValidator
			: public mocks::MockNotificationValidatorT<model::Notification>
			, public test::ParamsCapture<StatefulNotificationValidatorParams> {
	private:
		using BaseType = mocks::MockNotificationValidatorT<model::Notification>;

	public:
		using BaseType::MockNotificationValidatorT;

	public:
		validators::ValidationResult validate(
				const model::Notification& notification,
				const validators::ValidatorContext& context) const override {
			const_cast<MockCapturingStatefulNotificationValidator*>(this)->push(notification, context);
			return BaseType::validate(notification, context);
		}
	};
}}
