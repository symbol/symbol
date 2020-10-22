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

#include "FinalizationMessageTestUtils.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/crypto_voting/AggregateBmPrivateKeyTree.h"
#include "catapult/crypto_voting/VotingSigner.h"
#include "catapult/model/BlockUtils.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/test/nodeps/Random.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region finalization round + step identifier factories

	model::FinalizationRound CreateFinalizationRound(uint32_t epoch, uint32_t point) {
		return { FinalizationEpoch(epoch), FinalizationPoint(point) };
	}

	model::StepIdentifier CreateStepIdentifier(uint32_t epoch, uint32_t point, model::FinalizationStage stage) {
		return { FinalizationEpoch(epoch), FinalizationPoint(point), stage };
	}

	// endregion

	// region voting keys

	crypto::VotingKeyPair GenerateVotingKeyPair() {
		return crypto::VotingKeyPair::FromPrivate(crypto::VotingPrivateKey::Generate(RandomByte));
	}

	crypto::VotingKeyPair CopyKeyPair(const crypto::VotingKeyPair& votingKeyPair) {
		return crypto::VotingKeyPair::FromPrivate(crypto::VotingPrivateKey::FromBuffer(votingKeyPair.privateKey()));
	}

	// endregion

	// region message factories

	std::unique_ptr<model::FinalizationMessage> CreateMessage(FinalizationPoint point) {
		return CreateMessage({ FinalizationEpoch(), point, model::FinalizationStage::Count }, GenerateRandomByteArray<Hash256>());
	}

	std::unique_ptr<model::FinalizationMessage> CreateMessage(const model::FinalizationRound& round) {
		auto pMessage = CreateMessage(round.Point);
		pMessage->Data().StepIdentifier.Epoch = round.Epoch;
		return pMessage;
	}

	std::unique_ptr<model::FinalizationMessage> CreateMessage(FinalizationPoint point, Height height) {
		auto pMessage = CreateMessage(point);
		pMessage->Data().Height = height;
		return pMessage;
	}

	std::unique_ptr<model::FinalizationMessage> CreateMessage(const Hash256& hash) {
		return CreateMessage(GenerateRandomValue<Height>(), hash);
	}

	std::unique_ptr<model::FinalizationMessage> CreateMessage(Height height, const Hash256& hash) {
		auto stepIdentifier = model::StepIdentifier{
			FinalizationEpoch(static_cast<uint32_t>(Random())),
			FinalizationPoint(static_cast<uint32_t>(Random())),
			static_cast<model::FinalizationStage>(Random())
		};
		auto pMessage = CreateMessage(stepIdentifier, hash);
		pMessage->Data().Height = height;
		return pMessage;
	}

	namespace {
		uint32_t CalculateMessageSize(uint32_t numHashes) {
			return model::FinalizationMessage::MinSize() + numHashes * static_cast<uint32_t>(Hash256::Size);
		}

		uint32_t CalculateMessageSizeV1(uint32_t numHashes) {
			return CalculateMessageSize(numHashes) + SizeOf32<crypto::BmTreeSignatureV1>() - SizeOf32<crypto::BmTreeSignature>();
		}
	}

	std::unique_ptr<model::FinalizationMessage> CreateMessage(Height height, uint32_t numHashes) {
		uint32_t messageSize = CalculateMessageSize(numHashes);
		auto pMessage = utils::MakeUniqueWithSize<model::FinalizationMessage>(messageSize);
		FillWithRandomData({ reinterpret_cast<uint8_t*>(pMessage.get()), messageSize });
		pMessage->Size = messageSize;
		pMessage->FinalizationMessage_Reserved1 = 0;
		pMessage->SignatureScheme = 1;
		pMessage->Data().Version = model::FinalizationMessage::Current_Version;
		pMessage->Data().HashesCount = numHashes;
		pMessage->Data().Height = height;
		return pMessage;
	}

	std::unique_ptr<model::FinalizationMessage> CreateMessageV1(Height height, uint32_t numHashes) {
		uint32_t messageSize = CalculateMessageSizeV1(numHashes);
		auto pMessage = utils::MakeUniqueWithSize<model::FinalizationMessage>(messageSize);
		FillWithRandomData({ reinterpret_cast<uint8_t*>(pMessage.get()), messageSize });
		pMessage->Size = messageSize;
		pMessage->FinalizationMessage_Reserved1 = 0;
		pMessage->SignatureScheme = 0;
		pMessage->Data().Version = model::FinalizationMessage::Current_Version;
		pMessage->Data().HashesCount = numHashes;
		pMessage->Data().Height = height;
		return pMessage;
	}

	std::unique_ptr<model::FinalizationMessage> CreateMessage(const model::StepIdentifier& stepIdentifier, const Hash256& hash) {
		uint32_t messageSize = CalculateMessageSize(1);
		auto pMessage = utils::MakeUniqueWithSize<model::FinalizationMessage>(messageSize);
		pMessage->Size = messageSize;
		pMessage->FinalizationMessage_Reserved1 = 0;
		pMessage->SignatureScheme = 1;
		pMessage->Data().Version = model::FinalizationMessage::Current_Version;
		pMessage->Data().HashesCount = 1;
		pMessage->Data().StepIdentifier = stepIdentifier;

		FillWithRandomData(pMessage->Signature());
		*pMessage->HashesPtr() = hash;
		return pMessage;
	}

	// endregion

	// region multi-message factories

	std::vector<std::shared_ptr<model::FinalizationMessage>> CreatePrevoteMessages(
			FinalizationEpoch epoch,
			FinalizationPoint point,
			Height height,
			size_t numMessages,
			const Hash256* pHashes,
			size_t numHashes) {
		std::vector<std::shared_ptr<model::FinalizationMessage>> messages;
		for (auto i = 0u; i < numMessages; ++i) {
			auto pMessage = CreateMessage(height, static_cast<uint32_t>(numHashes));
			pMessage->Data().StepIdentifier = { epoch, point, model::FinalizationStage::Prevote };
			std::copy(pHashes, pHashes + numHashes, pMessage->HashesPtr());
			messages.push_back(std::move(pMessage));
		}

		return messages;
	}

	std::vector<std::shared_ptr<model::FinalizationMessage>> CreatePrecommitMessages(
			FinalizationEpoch epoch,
			FinalizationPoint point,
			Height height,
			size_t numMessages,
			const Hash256* pHashes,
			size_t index) {
		std::vector<std::shared_ptr<model::FinalizationMessage>> messages;
		for (auto i = 0u; i < numMessages; ++i) {
			auto pMessage = CreateMessage(height + Height(index), 1);
			pMessage->Data().StepIdentifier = { epoch, point, model::FinalizationStage::Precommit };
			*pMessage->HashesPtr() = pHashes[index];
			messages.push_back(std::move(pMessage));
		}

		return messages;
	}

	// endregion

	// region message utils

	namespace {
		RawBuffer ToBuffer(const uint64_t& value) {
			return { reinterpret_cast<const uint8_t*>(&value), sizeof(uint64_t) };
		}
	}

	void SignMessage(model::FinalizationMessage& message, const crypto::VotingKeyPair& votingKeyPair) {
		if (1 == message.SignatureScheme) {
			auto storage = mocks::MockSeekableMemoryStream();
			auto bmOptions = crypto::BmOptions{ { 0 }, { 15 } };
			auto bmPrivateKeyTree = crypto::BmPrivateKeyTree::Create(CopyKeyPair(votingKeyPair), storage, bmOptions);

			auto keyIdentifier = model::StepIdentifierToBmKeyIdentifier(message.Data().StepIdentifier);
			message.Signature() = bmPrivateKeyTree.sign(keyIdentifier, {
				reinterpret_cast<const uint8_t*>(&message) + model::FinalizationMessage::Header_Size,
				message.Size - model::FinalizationMessage::Header_Size
			});
		} else {
			static constexpr uint64_t Testnet_Dilution = 128;

			auto topKeyPair = GenerateVotingKeyPair();
			auto lowKeyPair = GenerateVotingKeyPair();

			auto& signature = message.SignatureV1();

			using VotingKeyV1 = decltype(crypto::BmTreeSignatureV1::ParentPublicKeySignaturePair::ParentPublicKey);
			using VotingSignatureV1 = decltype(crypto::BmTreeSignatureV1::ParentPublicKeySignaturePair::Signature);

			signature.Root.ParentPublicKey = votingKeyPair.publicKey().copyTo<VotingKeyV1>();
			signature.Top.ParentPublicKey = topKeyPair.publicKey().copyTo<VotingKeyV1>();
			signature.Bottom.ParentPublicKey = lowKeyPair.publicKey().copyTo<VotingKeyV1>();

			auto identifier = message.Data().StepIdentifier.Epoch.unwrap();

			crypto::VotingSignature componentSignature;
			crypto::Sign(votingKeyPair, { signature.Top.ParentPublicKey, ToBuffer(identifier / Testnet_Dilution) }, componentSignature);
			signature.Root.Signature = componentSignature.copyTo<VotingSignatureV1>();

			crypto::Sign(topKeyPair, { signature.Bottom.ParentPublicKey, ToBuffer(identifier % Testnet_Dilution) }, componentSignature);
			signature.Top.Signature = componentSignature.copyTo<VotingSignatureV1>();

			auto headerSize = model::FinalizationMessage::Header_Size
					+ sizeof(crypto::BmTreeSignatureV1) - sizeof(crypto::BmTreeSignature);
			auto messageBuffer = RawBuffer{ reinterpret_cast<const uint8_t*>(&message) + headerSize, message.Size - headerSize };
			crypto::Sign(lowKeyPair, messageBuffer, componentSignature);
			signature.Bottom.Signature = componentSignature.copyTo<VotingSignatureV1>();
		}
	}

	void AssertEqualMessage(
			const model::FinalizationMessage& expected,
			const model::FinalizationMessage& actual,
			const std::string& message) {
		ASSERT_EQ(expected.Size, actual.Size) << message;
		EXPECT_EQ_MEMORY(&expected, &actual, expected.Size) << message;
	}

	// endregion

	// region account state cache utils

	std::vector<AccountKeyPairDescriptor> AddAccountsWithBalances(
			cache::AccountStateCacheDelta& accountStateCacheDelta,
			Height height,
			MosaicId mosaicId,
			const std::vector<Amount>& balances) {
		std::vector<AccountKeyPairDescriptor> keyPairDescriptors;
		for (auto balance : balances) {
			keyPairDescriptors.emplace_back(GenerateVotingKeyPair());

			auto address = GenerateRandomByteArray<Address>();
			accountStateCacheDelta.addAccount(address, height);
			auto& accountState = accountStateCacheDelta.find(address).get();
			accountState.SupplementalPublicKeys.voting().add({
				keyPairDescriptors.back().VotingPublicKey,
				FinalizationEpoch(1),
				FinalizationEpoch(100)
			});
			accountState.Balances.credit(mosaicId, balance);
		}

		accountStateCacheDelta.updateHighValueAccounts(height);
		return keyPairDescriptors;
	}

	// endregion

	// region finalization context utils

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		std::vector<AccountKeyPairDescriptor> AddAccountsWithBalances(
				cache::AccountStateCache& cache,
				Height height,
				const std::vector<Amount>& balances) {
			auto delta = cache.createDelta();
			auto keyPairDescriptors = test::AddAccountsWithBalances(*delta, height, Harvesting_Mosaic_Id, balances);
			cache.commit();
			return keyPairDescriptors;
		}
	}

	std::pair<model::FinalizationContext, std::vector<AccountKeyPairDescriptor>> CreateFinalizationContext(
			const finalization::FinalizationConfiguration& config,
			FinalizationEpoch epoch,
			Height height,
			const std::vector<Amount>& balances) {
		auto accountStateCacheOptions = CreateDefaultAccountStateCacheOptions(MosaicId(1111), Harvesting_Mosaic_Id);
		accountStateCacheOptions.MinVoterBalance = Amount(2'000'000);

		cache::AccountStateCache accountStateCache(cache::CacheConfiguration(), accountStateCacheOptions);
		auto keyPairDescriptors = AddAccountsWithBalances(accountStateCache, height, balances);

		auto generationHash = GenerateRandomByteArray<GenerationHash>();
		auto finalizationContext = model::FinalizationContext(epoch, height, generationHash, config, *accountStateCache.createView());

		return std::make_pair(std::move(finalizationContext), std::move(keyPairDescriptors));
	}

	// endregion
}}
