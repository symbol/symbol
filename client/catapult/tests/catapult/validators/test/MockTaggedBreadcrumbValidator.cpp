#include "MockTaggedBreadcrumbValidator.h"

namespace catapult { namespace mocks {

	namespace {
		template<typename TTaggedNotification>
		class MockTaggedBreadcrumbValidator : public validators::stateful::NotificationValidatorT<TTaggedNotification> {
		public:
			MockTaggedBreadcrumbValidator(uint8_t tag, std::vector<uint16_t>& breadcrumbs, validators::ValidationResult result)
					: m_tag(tag)
					, m_breadcrumbs(breadcrumbs)
					, m_result(result)
					, m_name(std::to_string(m_tag))
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			validators::ValidationResult validate(
					const TTaggedNotification& notification,
					const validators::ValidatorContext&) const override {
				m_breadcrumbs.push_back(static_cast<uint16_t>(notification.Tag << 8 | m_tag));
				return m_result;
			}

		private:
			uint8_t m_tag;
			std::vector<uint16_t>& m_breadcrumbs;
			validators::ValidationResult m_result;
			std::string m_name;
		};
	}

	validators::stateful::NotificationValidatorPointerT<test::TaggedNotification> CreateTaggedBreadcrumbValidator(
			uint8_t tag,
			std::vector<uint16_t>& breadcrumbs,
			validators::ValidationResult result) {
		return std::make_unique<MockTaggedBreadcrumbValidator<test::TaggedNotification>>(tag, breadcrumbs, result);
	}

	validators::stateful::NotificationValidatorPointerT<test::TaggedNotification2> CreateTaggedBreadcrumbValidator2(
			uint8_t tag,
			std::vector<uint16_t>& breadcrumbs,
			validators::ValidationResult result) {
		return std::make_unique<MockTaggedBreadcrumbValidator<test::TaggedNotification2>>(tag, breadcrumbs, result);
	}
}}
