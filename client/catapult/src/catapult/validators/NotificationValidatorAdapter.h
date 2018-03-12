#pragma once
#include "ValidatorTypes.h"
#include "catapult/model/NotificationPublisher.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace validators {

	/// A stateless notification validator to entity validator adapter.
	/// \note This adapter intentionally only supports stateless validators.
	class NotificationValidatorAdapter : public stateless::EntityValidator {
	private:
		using NotificationValidatorPointer = std::unique_ptr<const stateless::NotificationValidator>;
		using NotificationPublisherPointer = std::unique_ptr<const model::NotificationPublisher>;

	public:
		/// Creates a new adapter around \a pValidator and \a pPublisher.
		NotificationValidatorAdapter(NotificationValidatorPointer&& pValidator, NotificationPublisherPointer&& pPublisher);

	public:
		const std::string& name() const override;

		ValidationResult validate(const model::WeakEntityInfo& entityInfo) const override;

	private:
		NotificationValidatorPointer m_pValidator;
		NotificationPublisherPointer m_pPublisher;
	};
}}
