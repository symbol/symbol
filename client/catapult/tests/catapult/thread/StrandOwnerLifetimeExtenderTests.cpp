#include "catapult/thread/StrandOwnerLifetimeExtender.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"

namespace catapult { namespace thread {

#define TEST_CLASS StrandOwnerLifetimeExtenderTests

	using BreadcrumbsType = std::vector<uint16_t>;

	namespace {
		class Owner {
		public:
			explicit Owner(BreadcrumbsType& breadcrumbs) : m_breadcrumbs(breadcrumbs)
			{}

		public:
			void addCrumbs(uint8_t id, uint8_t numCrumbs) {
				// add numCrumbs crumbs with a slight delay between each push back
				for (uint8_t i = 0; i < numCrumbs; ++i) {
					m_breadcrumbs.push_back(static_cast<uint16_t>(id << 8 | (i + 1)));
					test::Sleep(1);
				}
			}

		private:
			BreadcrumbsType& m_breadcrumbs;
		};

		using ExtenderType = StrandOwnerLifetimeExtender<Owner>;

		struct TestContext {
		public:
			std::shared_ptr<IoServiceThreadPool> pPool;
			boost::asio::strand Strand;
			std::shared_ptr<Owner> pOwner;
			ExtenderType Extender;
			BreadcrumbsType Breadcrumbs;

		public:
			TestContext()
					: pPool(test::CreateStartedIoServiceThreadPool())
					, Strand(boost::asio::strand(pPool->service()))
					, pOwner(std::make_shared<Owner>(Breadcrumbs))
					, Extender(Strand)
			{}
		};
	}

	TEST(TEST_CLASS, WrapExtendsLifetime) {
		// Arrange:
		TestContext context;

		// Act: create a wrapper that does NOT capture the owner shared_ptr
		auto& extender = context.Extender;
		auto addCrumbs = extender.wrap(context.pOwner, [&owner = *context.pOwner](auto id, auto numCrumbs) {
			owner.addCrumbs(static_cast<uint8_t>(id), static_cast<uint8_t>(numCrumbs));
		});

		// - destroy the owner
		context.pOwner.reset();

		// - invoke some work
		addCrumbs(0x70, 5);
		addCrumbs(0x10, 1);
		addCrumbs(0xF0, 3);

		// - wait for all work to complete
		context.pPool->join();

		// Assert: all breadcrumbs were pushed in order
		BreadcrumbsType expected{ 0x7001, 0x7002, 0x7003, 0x7004, 0x7005, 0x1001, 0xF001, 0xF002, 0xF003 };
		EXPECT_EQ(expected, context.Breadcrumbs);
		EXPECT_FALSE(!!context.pOwner); // sanity
	}

	TEST(TEST_CLASS, PostExtendsLifetime) {
		// Arrange:
		TestContext context;

		// Act: post some work that does NOT capture the owner shared_ptr
		context.Extender.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x70, 5); });
		context.Extender.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x10, 1); });
		context.Extender.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0xF0, 3); });

		// - destroy the owner
		context.pOwner.reset();

		// - wait for all work to complete
		context.pPool->join();

		// Assert: all breadcrumbs were pushed in order
		BreadcrumbsType expected{ 0x7001, 0x7002, 0x7003, 0x7004, 0x7005, 0x1001, 0xF001, 0xF002, 0xF003 };
		EXPECT_EQ(expected, context.Breadcrumbs);
		EXPECT_FALSE(!!context.pOwner); // sanity
	}

	TEST(TEST_CLASS, WrapAndPostAreProtectedByStrand) {
		// Arrange:
		TestContext context;

		// - create a wrapper
		auto& extender = context.Extender;
		auto addCrumbs = extender.wrap(context.pOwner, [&owner = *context.pOwner](auto id, auto numCrumbs) {
			owner.addCrumbs(static_cast<uint8_t>(id), static_cast<uint8_t>(numCrumbs));
		});

		// Act: post some work via post and addCrumbs, neither of which capture the owner shared_ptr
		context.Extender.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x70, 2); });
		addCrumbs(0x07, 2);
		context.Extender.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x10, 2); });
		addCrumbs(0xAA, 2);
		context.Extender.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0xF0, 2); });

		// - destroy the owner
		context.pOwner.reset();

		// - wait for all work to complete
		context.pPool->join();

		// Assert: all breadcrumbs were pushed in order
		BreadcrumbsType expected{ 0x7001, 0x7002, 0x0701, 0x0702, 0x1001, 0x1002, 0xAA01, 0xAA02, 0xF001, 0xF002 };
		EXPECT_EQ(expected, context.Breadcrumbs);
		EXPECT_FALSE(!!context.pOwner); // sanity
	}

	TEST(TEST_CLASS, StrandCanBeSharedBetweenInstances) {
		// Arrange: create three extenders
		TestContext context;
		ExtenderType extender2(context.Strand);
		ExtenderType extender3(context.Strand);

		// Act: post some work that does NOT capture the owner shared_ptr
		context.Extender.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x70, 5); });
		extender2.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x10, 1); });
		extender3.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0xF0, 3); });

		// - destroy the owner
		context.pOwner.reset();

		// - wait for all work to complete
		context.pPool->join();

		// Assert: all breadcrumbs were pushed in order even though multiple extenders were used
		BreadcrumbsType expected{ 0x7001, 0x7002, 0x7003, 0x7004, 0x7005, 0x1001, 0xF001, 0xF002, 0xF003 };
		EXPECT_EQ(expected, context.Breadcrumbs);
		EXPECT_FALSE(!!context.pOwner); // sanity
	}
}}
