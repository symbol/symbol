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
#include "catapult/crypto_voting/BmOptions.h"
#include "catapult/crypto_voting/VotingKeyPair.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Two-layer Bellare-Miner private key tree sizes.
	struct BmTreeSizes {
	private:
		static constexpr auto Bm_Public_Key_Size = VotingKey::Size;
		static constexpr auto Bm_Private_Key_Size = crypto::VotingPrivateKey::Size;
		static constexpr auto Bm_Signature_Size = crypto::VotingSignature::Size;

		static constexpr auto File_Header_Size = sizeof(crypto::BmOptions) + 2 * sizeof(crypto::BmKeyIdentifier);
		static constexpr auto Level_Header_Size = Bm_Public_Key_Size + 2 * sizeof(uint64_t);

	public:
		static constexpr auto Level_Entry_Size = Bm_Private_Key_Size + Bm_Signature_Size;

	public:
		/// Calculates the start offset of level one keys.
		static constexpr auto CalculateLevelOnePayloadStart(size_t) {
			return File_Header_Size + Level_Header_Size;
		}

		/// Calculates the full level one size given \a levelOneSize.
		static constexpr auto CalculateFullLevelOneSize(size_t levelOneSize) {
			return File_Header_Size + Level_Header_Size + levelOneSize * Level_Entry_Size;
		}
	};

	/// Asserts that \a expected and \a actual are equal.
	void AssertOptions(const crypto::BmOptions& expected, const crypto::BmOptions& actual);

	/// Asserts that \a buffer has \a keysCount keys starting at \a offset such that only those with
	/// an index in \a expectedZeroedIndexes are zeroed.
	/// \a message is prefixed to assert messages.
	void AssertZeroedKeys(
			const std::vector<uint8_t>& buffer,
			size_t offset,
			size_t keysCount,
			const std::unordered_set<size_t>& expectedZeroedIndexes,
			const std::string& message);

	/// Asserts that \a tree can sign a message with identifier \a keyIdentifier.
	template<typename TTree>
	void AssertCanSign(TTree& tree, const crypto::BmKeyIdentifier& keyIdentifier) {
		// Arrange:
		auto messageBuffer = GenerateRandomArray<10>();

		// Act:
		auto canSign = tree.canSign(keyIdentifier);
		auto signature = tree.sign(keyIdentifier, messageBuffer);

		// Assert:
		EXPECT_TRUE(canSign) << keyIdentifier;
		EXPECT_TRUE(Verify(signature, keyIdentifier, messageBuffer)) << keyIdentifier;
	}

	/// Asserts that \a tree cannot sign a message with identifier \a keyIdentifier.
	template<typename TTree>
	void AssertCannotSign(TTree& tree, const crypto::BmKeyIdentifier& keyIdentifier) {
		// Arrange:
		auto messageBuffer = GenerateRandomArray<10>();

		// Act:
		auto canSign = tree.canSign(keyIdentifier);
		EXPECT_THROW(tree.sign(keyIdentifier, messageBuffer), catapult_invalid_argument) << keyIdentifier;

		// Assert:
		EXPECT_FALSE(canSign) << keyIdentifier;
	}
}}
