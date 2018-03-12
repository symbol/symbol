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

		static void CheckFields(size_t additionalSize, const TTransaction& transaction) {
			ASSERT_EQ(sizeof(TTransaction) + additionalSize, transaction.Size);

			EXPECT_EQ(Signature{}, transaction.Signature);
			EXPECT_EQ(Amount(0), transaction.Fee);
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

		static void CheckFields(size_t additionalSize, const TTransaction& transaction) {
			ASSERT_EQ(sizeof(TTransaction) + additionalSize, transaction.Size);
		}
	};
}}
