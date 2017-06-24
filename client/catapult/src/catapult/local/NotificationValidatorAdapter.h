#pragma once
#include "catapult/model/NotificationPublisher.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace local {

	/// A stateless notification validator to entity validator adapter.
	/// \note This adapter intentionally only supports stateless validators.
	class NotificationValidatorAdapter : public validators::stateless::EntityValidator {
	private:
		using NotificationValidator = validators::stateless::NotificationValidator;
		using NotificationValidatorPointer = std::unique_ptr<const NotificationValidator>;

	public:
		/// Creates a new adapter around \a pValidator given \a transactionRegistry.
		explicit NotificationValidatorAdapter(
				const model::TransactionRegistry& transactionRegistry,
				NotificationValidatorPointer&& pValidator);

	public:
		const std::string& name() const override;

		validators::ValidationResult validate(const model::WeakEntityInfo& entityInfo) const override;

	private:
		NotificationValidatorPointer m_pValidator;
		std::unique_ptr<model::NotificationPublisher> m_pPub;
	};
}}
