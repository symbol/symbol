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

#include "catapult/thread/StrandOwnerLifetimeExtender.h"
#include "catapult/thread/IoThreadPool.h"
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
			std::unique_ptr<IoThreadPool> pPool;
			boost::asio::io_context::strand Strand;
			std::shared_ptr<Owner> pOwner;
			ExtenderType Extender;
			BreadcrumbsType Breadcrumbs;

		public:
			TestContext()
					: pPool(test::CreateStartedIoThreadPool())
					, Strand(boost::asio::io_context::strand(pPool->ioContext()))
					, pOwner(std::make_shared<Owner>(Breadcrumbs))
					, Extender(Strand)
			{}
		};

		template<typename TFunc, typename... TArgs>
		void DispatchWrappedFunction(TFunc func, TArgs&&... args) {
			// simulate deprecated strand::wrap
			boost::asio::dispatch(boost::asio::get_associated_executor(func), std::bind(func, std::forward<TArgs>(args)...));
		}
	}

	TEST(TEST_CLASS, WrapExtendsLifetime) {
		// Arrange:
		TestContext context;

		// Act: create a wrapper that does NOT capture the owner shared_ptr
		auto addCrumbs = context.Extender.wrap(context.pOwner, [&owner = *context.pOwner](auto id, auto numCrumbs) {
			owner.addCrumbs(static_cast<uint8_t>(id), static_cast<uint8_t>(numCrumbs));
		});

		// - destroy the owner
		context.pOwner.reset();

		// - invoke some work
		DispatchWrappedFunction(addCrumbs, 0x70, 5);
		DispatchWrappedFunction(addCrumbs, 0x10, 1);
		DispatchWrappedFunction(addCrumbs, 0xF0, 3);

		// - wait for all work to complete
		context.pPool->join();

		// Assert: all breadcrumbs were pushed in order
		BreadcrumbsType expected{ 0x7001, 0x7002, 0x7003, 0x7004, 0x7005, 0x1001, 0xF001, 0xF002, 0xF003 };
		EXPECT_EQ(expected, context.Breadcrumbs);
		EXPECT_FALSE(!!context.pOwner); // sanity
	}

	TEST(TEST_CLASS, DispatchExtendsLifetime) {
		// Arrange:
		TestContext context;

		// Act: dispatch some work that does NOT capture the owner shared_ptr
		context.Extender.dispatch(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x70, 5); });
		context.Extender.dispatch(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x10, 1); });
		context.Extender.dispatch(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0xF0, 3); });

		// - destroy the owner
		context.pOwner.reset();

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

	TEST(TEST_CLASS, AllOperationsAreProtectedByStrand) {
		// Arrange:
		TestContext context;

		// - create a wrapper
		auto addCrumbs = context.Extender.wrap(context.pOwner, [&owner = *context.pOwner](auto id, auto numCrumbs) {
			owner.addCrumbs(static_cast<uint8_t>(id), static_cast<uint8_t>(numCrumbs));
		});

		// Act: add some work via post, dispatch and addCrumbs; none of which capture the owner shared_ptr
		context.Extender.post(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x70, 2); });
		DispatchWrappedFunction(addCrumbs, 0x07, 2);
		context.Extender.dispatch(context.pOwner, [](const auto& pOwner) { pOwner->addCrumbs(0x10, 2); });
		DispatchWrappedFunction(addCrumbs, 0xAA, 2);
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

	TEST(TEST_CLASS, PostDispatchHaveExpectedBehaviorWhenCalledWithinStrand) {
		// Arrange:
		TestContext context;

		// Act: post and dispatch within the strand
		DispatchWrappedFunction(context.Extender.wrap(context.pOwner, [&context, pOwnerOuter = context.pOwner]() {
			context.Extender.post(pOwnerOuter, [](const auto& pOwner) { pOwner->addCrumbs(0x80, 2); });
			context.Extender.dispatch(pOwnerOuter, [](const auto& pOwner) { pOwner->addCrumbs(0x90, 2); });
			context.Extender.post(pOwnerOuter, [](const auto& pOwner) { pOwner->addCrumbs(0xA0, 2); });
			context.Extender.dispatch(pOwnerOuter, [](const auto& pOwner) { pOwner->addCrumbs(0xB0, 2); });
		}));

		// - destroy the owner
		context.pOwner.reset();

		// - wait for all work to complete
		context.pPool->join();

		// Assert: all breadcrumbs were pushed in expected order
		// dispatch should execute immediately when called within a strand, while post should wait for the strand to be available
		BreadcrumbsType expected{ 0x9001, 0x9002, 0xB001, 0xB002, 0x8001, 0x8002, 0xA001, 0xA002 };
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
