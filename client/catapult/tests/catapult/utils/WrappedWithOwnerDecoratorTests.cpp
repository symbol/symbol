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

#include "catapult/utils/WrappedWithOwnerDecorator.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS WrappedWithOwnerDecoratorTests

	namespace {
		struct BasicTraits {
			template<typename... TArgs>
			using DecoratorType = WrappedWithOwnerDecorator<TArgs...>;
		};

		struct ResettableTraits {
			template<typename... TArgs>
			using DecoratorType = ResettableWrappedWithOwnerDecorator<TArgs...>;
		};

		template<typename TTraits, typename TOwner, typename THandler>
		auto Decorate(const std::shared_ptr<TOwner>& pOwner, THandler handler) {
			return typename TTraits::template DecoratorType<THandler>(pOwner, handler);
		}
	}

#define DECORATOR_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BasicTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Resettable) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ResettableTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	DECORATOR_BASED_TEST(DecoratorForwardsToCallable) {
		// Arrange: wrap a decorator around a lambda that depends on pOwner
		auto pOwner = std::make_shared<uint32_t>(7);
		auto counter = 0u;
		auto decorator = Decorate<TTraits>(pOwner, [owner = *pOwner, &counter](uint32_t multiple) {
			counter += owner * multiple;
		});

		// Act: invoke the lambda
		decorator(static_cast<uint32_t>(3));
		decorator(static_cast<uint32_t>(2));

		// Assert: the function was called and decorator has kept the owner alive
		EXPECT_EQ(35u, counter);
		EXPECT_EQ(2, decorator.owner().use_count());
	}

	DECORATOR_BASED_TEST(DecoratorForwardsResultsFromCallabale) {
		// Arrange: wrap a decorator around a lambda that depends on pOwner
		auto pOwner = std::make_shared<uint32_t>(7);
		auto decorator = Decorate<TTraits>(pOwner, [owner = *pOwner](uint32_t multiple) {
			return owner * multiple;
		});

		// Act: invoke the lambda
		auto result1 = decorator(static_cast<uint32_t>(3));
		auto result2 = decorator(static_cast<uint32_t>(2));

		// Assert: the function was called and decorator has kept the owner alive
		EXPECT_EQ(21u, result1);
		EXPECT_EQ(14u, result2);
		EXPECT_EQ(2, decorator.owner().use_count());
	}

	DECORATOR_BASED_TEST(DecoratorExtendsLifetime) {
		// Arrange: wrap a decorator around a lambda that depends on pOwner
		auto pOwner = std::make_shared<uint32_t>(7);
		auto counter = 0u;
		auto decorator = Decorate<TTraits>(pOwner, [owner = *pOwner, &counter](uint32_t multiple) {
			counter += owner * multiple;
		});

		// Act: reset the local owner pointer
		EXPECT_EQ(2, decorator.owner().use_count());
		pOwner.reset();
		EXPECT_EQ(1, decorator.owner().use_count());

		// - invoke the lambda
		decorator(static_cast<uint32_t>(3));

		// Assert: the function was called and decorator has kept the owner alive
		EXPECT_EQ(21u, counter);
	}

	namespace {
		// RAII class that updates a counter in its destructor
		class DestructingCounterUpdater {
		public:
			DestructingCounterUpdater(uint32_t& owner, uint32_t& counter)
					: m_owner(owner)
					, m_counter(counter)
					, m_multiple(0)
			{}

			~DestructingCounterUpdater() {
				// updating the counter in the destructor implies that both the counter and owner must still be valid
				m_counter = m_owner * m_multiple;
			}

		public:
			void setMultiple(uint32_t multiple) {
				m_multiple = multiple;
			}

		private:
			uint32_t& m_owner;
			uint32_t& m_counter;
			uint32_t m_multiple;
		};
	}

	DECORATOR_BASED_TEST(DecoratorDestroysOwnerLast) {
		// Arrange: wrap a decorator around a lambda that depends on pOwner
		auto counter = 0u;
		{
			auto pOwner = std::make_shared<uint32_t>(7);
			auto pUpdater = std::make_shared<DestructingCounterUpdater>(*pOwner, counter);
			auto decorator = Decorate<TTraits>(pOwner, [pUpdater = std::move(pUpdater)](uint32_t multiple) {
				pUpdater->setMultiple(multiple);
			});

			// Act: make the decorator hold the last reference to the owner
			pOwner.reset();
			EXPECT_EQ(1, decorator.owner().use_count());

			// - invoke the lambda
			decorator(static_cast<uint32_t>(3));

			// Sanity: the counter was not yet updated
			EXPECT_EQ(0u, counter);
		}

		// Assert: the function was called and decorator has kept the owner alive
		//         shutdown order must be: decorator::handler -> pUpdater(references owner) -> decorator::owner
		EXPECT_EQ(21u, counter);
	}

	TEST(TEST_CLASS, ResettableDecoratorCanBeReset) {
		// Arrange: wrap a decorator around a lambda that depends on pOwner
		auto pOwner = std::make_shared<uint32_t>(7);
		auto counter = 0u;
		auto decorator = Decorate<ResettableTraits>(pOwner, consumer<uint32_t>([pOwner, &counter](uint32_t multiple) {
			counter += *pOwner * multiple;
		}));

		// Sanity: local, captured owner in lambda, captured owner in decorator
		EXPECT_EQ(3, pOwner.use_count());

		// Act: reset the decorator:
		decorator.reset();

		// Assert: only the local reference remains
		EXPECT_EQ(1, pOwner.use_count());
	}
}}
