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
#include "finalization/src/model/FinalizationContext.h"
#include "finalization/src/model/FinalizationMessage.h"
#include "catapult/model/FinalizationRound.h"
#include <memory>

namespace catapult {
	namespace cache { class AccountStateCacheDelta; }
	namespace test { struct AccountKeyPairDescriptor; }
}

namespace catapult { namespace test {

	// region finalization round + step identifier factories

	/// Creates a finalization round with the specified \a epoch and \a point.
	model::FinalizationRound CreateFinalizationRound(uint32_t epoch, uint32_t point);

	/// Creates a step identifier with the specified \a epoch, \a point and \a stage.
	model::StepIdentifier CreateStepIdentifier(uint32_t epoch, uint32_t point, model::FinalizationStage stage);

	// endregion

	// region voting keys

	/// Generates a random voting key pair.
	crypto::VotingKeyPair GenerateVotingKeyPair();

	/// Copies a given \a votingKeyPair.
	crypto::VotingKeyPair CopyKeyPair(const crypto::VotingKeyPair& votingKeyPair);

	// endregion

	// region message factories

	/// Creates a finalization message with one hash for \a point.
	std::unique_ptr<model::FinalizationMessage> CreateMessage(FinalizationPoint point);

	/// Creates a finalization message with one hash for \a round.
	std::unique_ptr<model::FinalizationMessage> CreateMessage(const model::FinalizationRound& round);

	/// Creates a finalization message with one hash for \a point at \a height.
	std::unique_ptr<model::FinalizationMessage> CreateMessage(FinalizationPoint point, Height height);

	/// Creates a finalization message around one \a hash.
	std::unique_ptr<model::FinalizationMessage> CreateMessage(const Hash256& hash);

	/// Creates a finalization message around one \a hash at \a height.
	std::unique_ptr<model::FinalizationMessage> CreateMessage(Height height, const Hash256& hash);

	/// Creates a finalization message around \a numHashes hashes at \a height.
	std::unique_ptr<model::FinalizationMessage> CreateMessage(Height height, uint32_t numHashes);

	/// Creates a finalization message around \a numHashes hashes at \a height.
	std::unique_ptr<model::FinalizationMessage> CreateMessageV1(Height height, uint32_t numHashes);

	/// Creates a finalization message with \a stepIdentifier and one \a hash.
	std::unique_ptr<model::FinalizationMessage> CreateMessage(const model::StepIdentifier& stepIdentifier, const Hash256& hash);

	// endregion

	// region multi-message factories

	/// Creates \a numMessages prevote messages for the specified hashes (\a numHashes starting at \a pHashes) with specified starting
	/// \a height, \a epoch and \a point.
	std::vector<std::shared_ptr<model::FinalizationMessage>> CreatePrevoteMessages(
			FinalizationEpoch epoch,
			FinalizationPoint point,
			Height height,
			size_t numMessages,
			const Hash256* pHashes,
			size_t numHashes);

	/// Creates \a numMessages precommit messages for the specified hash (at \a index in \a pHashes) with specified starting
	/// \a height, \a epoch and \a point.
	std::vector<std::shared_ptr<model::FinalizationMessage>> CreatePrecommitMessages(
			FinalizationEpoch epoch,
			FinalizationPoint point,
			Height height,
			size_t numMessages,
			const Hash256* pHashes,
			size_t index);

	// endregion

	// region message utils

	/// Signs \a message with \a votingKeyPair.
	void SignMessage(model::FinalizationMessage& message, const crypto::VotingKeyPair& votingKeyPair);

	/// Asserts that \a expected and \a actual are equal with optional \a message.
	void AssertEqualMessage(
			const model::FinalizationMessage& expected,
			const model::FinalizationMessage& actual,
			const std::string& message = std::string());

	// endregion

	// region account state cache utils

	/// Account descriptor that contains voting key pairs.
	struct AccountKeyPairDescriptor {
	public:
		/// Creates a descriptor around \a votingKeyPair.
		explicit AccountKeyPairDescriptor(crypto::VotingKeyPair&& votingKeyPair)
				: VotingKeyPair(std::move(votingKeyPair))
				, VotingPublicKey(VotingKeyPair.publicKey().copyTo<VotingKey>())
		{}

	public:
		/// Voting key pair.
		crypto::VotingKeyPair VotingKeyPair;

		/// Voting public key.
		VotingKey VotingPublicKey;
	};

	/// Adds accounts with specified \a balances of \a mosaicId to \a accountStateCacheDelta at \a height.
	std::vector<AccountKeyPairDescriptor> AddAccountsWithBalances(
			cache::AccountStateCacheDelta& accountStateCacheDelta,
			Height height,
			MosaicId mosaicId,
			const std::vector<Amount>& balances);

	// endregion

	// region finalization context utils

	/// Creates a finalization context for \a epoch at \a height with specified account \a balances given \a config.
	std::pair<model::FinalizationContext, std::vector<AccountKeyPairDescriptor>> CreateFinalizationContext(
			const finalization::FinalizationConfiguration& config,
			FinalizationEpoch epoch,
			Height height,
			const std::vector<Amount>& balances);

	// endregion
}}
