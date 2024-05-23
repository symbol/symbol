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

#include "catapult/utils/MemoryUtils.h"
#include "src/model/HashLockTransaction.h"
#include "tests/TestHarness.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/Alignment.h"

namespace catapult {
namespace model {

    using TransactionType = HashLockTransaction;

#define TEST_CLASS HashLockTransactionTests

    // region size + alignment + properties

#define TRANSACTION_FIELDS FIELD(Mosaic) FIELD(Duration) FIELD(Hash)

    namespace {
        template <typename T>
        void AssertTransactionHasExpectedSize(size_t baseSize)
        {
            // Arrange:
            auto expectedSize = baseSize;

#define FIELD(X) expectedSize += SizeOf32<decltype(T::X)>();
            TRANSACTION_FIELDS
#undef FIELD

            // Assert:
            EXPECT_EQ(expectedSize, sizeof(T));
            EXPECT_EQ(baseSize + 56u, sizeof(T));
        }

        template <typename T>
        void AssertTransactionHasProperAlignment()
        {
#define FIELD(X) EXPECT_ALIGNED(T, X);
            TRANSACTION_FIELDS
#undef FIELD
        }

        template <typename T>
        void AssertTransactionHasExpectedProperties()
        {
            // Assert:
            EXPECT_EQ(Entity_Type_Hash_Lock, T::Entity_Type);
            EXPECT_EQ(1u, T::Current_Version);
        }
    }

#undef TRANSACTION_FIELDS

    ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(HashLock)

    // endregion

    // region CalculateRealSize

    TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues)
    {
        // Arrange:
        HashLockTransaction transaction;
        transaction.Size = 0;

        // Act:
        auto realSize = HashLockTransaction::CalculateRealSize(transaction);

        // Assert:
        EXPECT_EQ(sizeof(HashLockTransaction), realSize);
    }

    // endregion
}
}
