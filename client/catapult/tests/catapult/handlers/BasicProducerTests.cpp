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

#include "catapult/handlers/BasicProducer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS BasicProducerTests

	namespace {
		class IntProducer : public BasicProducer<std::vector<int>> {
		public:
			using BasicProducer<std::vector<int>>::BasicProducer;

		public:
			std::shared_ptr<int> operator()() {
				return next([](auto i) { return std::make_shared<int>(2 * i); });
			}
		};

		auto ProduceAll(IntProducer& producer) {
			std::vector<int> producedValues;
			for (;;) {
				auto pInt = producer();
				if (!pInt)
					break;

				producedValues.push_back(*pInt);
			}

			return producedValues;
		}
	}

	TEST(TEST_CLASS, CanProduceZeroEntities) {
		// Arrange:
		auto seedValues = std::vector<int>();
		IntProducer producer(seedValues);

		// Act:
		auto producedValues = ProduceAll(producer);

		// Assert:
		EXPECT_EQ(std::vector<int>(), producedValues);
	}

	TEST(TEST_CLASS, CanProduceSingleEntity) {
		// Arrange:
		auto seedValues = std::vector<int>{ 7 };
		IntProducer producer(seedValues);

		// Act:
		auto producedValues = ProduceAll(producer);

		// Assert:
		EXPECT_EQ(std::vector<int>({ 14 }), producedValues);
	}

	TEST(TEST_CLASS, CanProduceMultipleEntiies) {
		// Arrange:
		auto seedValues = std::vector<int>{ 7, 11, 6 };
		IntProducer producer(seedValues);

		// Act:
		auto producedValues = ProduceAll(producer);

		// Assert:
		EXPECT_EQ(std::vector<int>({ 14, 22, 12 }), producedValues);
	}
}}
