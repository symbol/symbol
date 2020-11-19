/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
