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

#pragma once
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	template<typename TTransaction>
	struct RegularTransactionTraits {
	public:
		template<typename TBuilder>
		static auto InvokeBuilder(TBuilder& builder) {
			return builder.build();
		}

		template<typename TBuilder>
		static void CheckBuilderSize(size_t additionalSize, const TBuilder& builder) {
			EXPECT_EQ(sizeof(TTransaction) + additionalSize, builder.size());
		}

		static void CheckFields(size_t additionalSize, const TTransaction& transaction) {
			ASSERT_EQ(sizeof(TTransaction) + additionalSize, transaction.Size);

			EXPECT_EQ(Signature(), transaction.Signature);
			EXPECT_EQ(Amount(0), transaction.MaxFee);
			EXPECT_EQ(Timestamp(0), transaction.Deadline);
		}
	};

	template<typename TTransaction>
	struct EmbeddedTransactionTraits {
	public:
		template<typename TBuilder>
		static auto InvokeBuilder(TBuilder& builder) {
			return builder.buildEmbedded();
		}

		template<typename TBuilder>
		static void CheckBuilderSize(size_t, const TBuilder&)
		{}

		static void CheckFields(size_t additionalSize, const TTransaction& transaction) {
			ASSERT_EQ(sizeof(TTransaction) + additionalSize, transaction.Size);
		}
	};
}}
