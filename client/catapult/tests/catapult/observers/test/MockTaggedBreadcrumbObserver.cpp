#include "MockTaggedBreadcrumbObserver.h"

namespace catapult { namespace mocks {

	namespace {
		template<typename TTaggedNotification>
		class MockTaggedBreadcrumbObserver : public observers::NotificationObserverT<TTaggedNotification> {
		public:
			MockTaggedBreadcrumbObserver(uint8_t tag, std::vector<uint16_t>& breadcrumbs)
					: m_tag(tag)
					, m_breadcrumbs(breadcrumbs)
					, m_name(std::to_string(m_tag))
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			void notify(const TTaggedNotification& notification, const observers::ObserverContext&) const override {
				m_breadcrumbs.push_back(static_cast<uint16_t>(notification.Tag << 8 | m_tag));
			}

		private:
			uint8_t m_tag;
			std::vector<uint16_t>& m_breadcrumbs;
			std::string m_name;
		};
	}

	observers::NotificationObserverPointerT<test::TaggedNotification> CreateTaggedBreadcrumbObserver(
			uint8_t tag,
			std::vector<uint16_t>& breadcrumbs) {
		return std::make_unique<MockTaggedBreadcrumbObserver<test::TaggedNotification>>(tag, breadcrumbs);
	}

	observers::NotificationObserverPointerT<test::TaggedNotification2> CreateTaggedBreadcrumbObserver2(
			uint8_t tag,
			std::vector<uint16_t>& breadcrumbs) {
		return std::make_unique<MockTaggedBreadcrumbObserver<test::TaggedNotification2>>(tag, breadcrumbs);
	}
}}
