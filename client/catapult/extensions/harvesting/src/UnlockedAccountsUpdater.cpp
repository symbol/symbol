/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "UnlockedAccountsUpdater.h"
#include "UnlockedAccounts.h"
#include "UnlockedFileQueueConsumer.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/ImportanceView.h"
#include "catapult/io/RawFile.h"
#include "catapult/model/Address.h"

namespace catapult { namespace harvesting {

	namespace {
		bool AddToUnlocked(UnlockedAccounts& unlockedAccounts, BlockGeneratorAccountDescriptor&& descriptor) {
			auto signingPublicKey = descriptor.signingKeyPair().publicKey();
			auto addResult = unlockedAccounts.modifier().add(std::move(descriptor));
			if (UnlockedAccountsAddResult::Success_New == addResult) {
				CATAPULT_LOG(important) << "added NEW account " << signingPublicKey;
				return true;
			}

			return false;
		}

		bool RemoveFromUnlocked(UnlockedAccounts& unlockedAccounts, const Key& publicKey) {
			if (unlockedAccounts.modifier().remove(publicKey)) {
				CATAPULT_LOG(info) << "removed account " << publicKey;
				return true;
			}

			return false;
		}

		// region DescriptorProcessor

		class DescriptorProcessor {
		public:
			DescriptorProcessor(
					const Key& nodePublicKey,
					const cache::CatapultCache& cache,
					UnlockedAccounts& unlockedAccounts,
					UnlockedAccountsStorage& storage)
					: m_nodePublicKey(nodePublicKey)
					, m_cache(cache)
					, m_unlockedAccounts(unlockedAccounts)
					, m_storage(storage)
					, m_hasAnyRemoval(false)
			{}

		public:
			bool hasAnyRemoval() const {
				return m_hasAnyRemoval;
			}

		public:
			void operator()(const HarvestRequest& harvestRequest, BlockGeneratorAccountDescriptor&& descriptor) {
				if (HarvestRequestOperation::Add == harvestRequest.Operation)
					add(harvestRequest, std::move(descriptor));
				else
					remove(GetRequestIdentifier(harvestRequest), descriptor.signingKeyPair().publicKey());
			}

			size_t pruneUnlockedAccounts() {
				auto cacheView = m_cache.createView();

				size_t numPrunedAccounts = 0;
				auto height = cacheView.height() + Height(1);
				m_unlockedAccounts.modifier().removeIf([this, height, &cacheView, &numPrunedAccounts](const auto& descriptor) {
					auto readOnlyAccountStateCache = cache::ReadOnlyAccountStateCache(cacheView.sub<cache::AccountStateCache>());
					cache::ImportanceView view(readOnlyAccountStateCache);

					auto address = model::PublicKeyToAddress(
							descriptor.signingKeyPair().publicKey(),
							readOnlyAccountStateCache.networkIdentifier());
					auto shouldPruneAccount = !view.canHarvest(address, height);

					if (!shouldPruneAccount) {
						auto remoteAccountStateIter = readOnlyAccountStateCache.find(address);
						if (state::AccountType::Remote == remoteAccountStateIter.get().AccountType) {
							auto mainAccountStateIter = readOnlyAccountStateCache.find(GetLinkedPublicKey(remoteAccountStateIter.get()));
							shouldPruneAccount = !isMainAccountEligibleForDelegation(mainAccountStateIter.get(), descriptor);
						}
					}

					if (shouldPruneAccount)
						++numPrunedAccounts;

					return shouldPruneAccount;
				});

				return numPrunedAccounts;
			}

		private:
			void add(const HarvestRequest& harvestRequest, BlockGeneratorAccountDescriptor&& descriptor) {
				if (!isMainAccountEligibleForDelegation(harvestRequest.MainAccountPublicKey, descriptor))
					return;

				auto harvesterSigningPublicKey = descriptor.signingKeyPair().publicKey();
				auto requestIdentifier = GetRequestIdentifier(harvestRequest);
				if (!m_storage.contains(requestIdentifier) && AddToUnlocked(m_unlockedAccounts, std::move(descriptor)))
					m_storage.add(requestIdentifier, harvestRequest.EncryptedPayload, harvesterSigningPublicKey);
			}

			void remove(HarvestRequestIdentifier requestIdentifier, const Key& harvesterSigningPublicKey) {
				RemoveFromUnlocked(m_unlockedAccounts, harvesterSigningPublicKey);
				m_storage.remove(requestIdentifier);
				m_hasAnyRemoval = true;
			}

			bool isMainAccountEligibleForDelegation(const Key& mainAccountPublicKey, const BlockGeneratorAccountDescriptor& descriptor) {
				auto cacheView = m_cache.createView();
				auto readOnlyAccountStateCache = cache::ReadOnlyAccountStateCache(cacheView.sub<cache::AccountStateCache>());
				auto accountStateIter = readOnlyAccountStateCache.find(mainAccountPublicKey);
				if (!accountStateIter.tryGet()) {
					CATAPULT_LOG(warning) << "rejecting delegation from " << mainAccountPublicKey << ": unknown main account";
					return false;
				}

				return isMainAccountEligibleForDelegation(accountStateIter.get(), descriptor);
			}

			bool isMainAccountEligibleForDelegation(
					const state::AccountState& accountState,
					const BlockGeneratorAccountDescriptor& descriptor) {
				if (GetLinkedPublicKey(accountState) != descriptor.signingKeyPair().publicKey()) {
					CATAPULT_LOG(warning) << "rejecting delegation from " << accountState.PublicKey << ": invalid signing public key";
					return false;
				}

				if (GetVrfPublicKey(accountState) != descriptor.vrfKeyPair().publicKey()) {
					CATAPULT_LOG(warning) << "rejecting delegation from " << accountState.PublicKey << ": invalid vrf public key";
					return false;
				}

				if (GetNodePublicKey(accountState) != m_nodePublicKey) {
					CATAPULT_LOG(warning) << "rejecting delegation from " << accountState.PublicKey << ": invalid node public key";
					return false;
				}

				return true;
			}

		private:
			Key m_nodePublicKey;
			const cache::CatapultCache& m_cache;
			UnlockedAccounts& m_unlockedAccounts;
			UnlockedAccountsStorage& m_storage;
			bool m_hasAnyRemoval;
		};

		// endregion
	}

	UnlockedAccountsUpdater::UnlockedAccountsUpdater(
			const cache::CatapultCache& cache,
			UnlockedAccounts& unlockedAccounts,
			const crypto::KeyPair& encryptionKeyPair,
			const config::CatapultDataDirectory& dataDirectory)
			: m_cache(cache)
			, m_unlockedAccounts(unlockedAccounts)
			, m_encryptionKeyPair(encryptionKeyPair)
			, m_dataDirectory(dataDirectory)
			, m_harvestersFilename(m_dataDirectory.rootDir().file("harvesters.dat"))
			, m_unlockedAccountsStorage(m_harvestersFilename)
	{}

	void UnlockedAccountsUpdater::load() {
		// load account descriptors
		m_unlockedAccountsStorage.load(m_encryptionKeyPair, [&unlockedAccounts = m_unlockedAccounts](auto&& descriptor) {
			AddToUnlocked(unlockedAccounts, std::move(descriptor));
		});
	}

	void UnlockedAccountsUpdater::update() {
		// 1. process queued accounts
		DescriptorProcessor processor(m_encryptionKeyPair.publicKey(), m_cache, m_unlockedAccounts, m_unlockedAccountsStorage);
		UnlockedFileQueueConsumer(m_dataDirectory.dir("transfer_message"), m_encryptionKeyPair, std::ref(processor));

		// 2. prune accounts that are not eligible to harvest the next block
		auto numPrunedAccounts = processor.pruneUnlockedAccounts();

		// 3. save accounts
		if (numPrunedAccounts > 0 || processor.hasAnyRemoval()) {
			auto view = m_unlockedAccounts.view();
			m_unlockedAccountsStorage.save([&view](const auto& harvesterSigningPublicKey) {
				return view.contains(harvesterSigningPublicKey);
			});
		}
	}
}}
