#include "DispatcherTestUtils.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/BlockDifficultyCache.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/BlockUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"

namespace catapult { namespace test {

	void InitializeCatapultCacheForDispatcherTests(cache::CatapultCache& cache, const crypto::KeyPair& signer) {
		// - create the delta
		auto delta = cache.createDelta();

		// - set a difficulty for the nemesis block
		delta.sub<cache::BlockDifficultyCache>().insert(Height(1), Timestamp(0), Difficulty());

		// - add a balance and importance for the signer
		auto pState = delta.sub<cache::AccountStateCache>().addAccount(signer.publicKey(), Height(1));
		pState->Balances.credit(Xem_Id, Amount(1'000'000'000'000));
		pState->ImportanceInfo.set(Importance(1'000'000'000), model::ImportanceHeight(1));

		// - commit all changes
		cache.commit(Height(1));
	}

	std::shared_ptr<model::Block> CreateValidBlockForDispatcherTests(const crypto::KeyPair& signer) {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		mocks::MemoryBasedStorage storage;
		auto pNemesisBlockElement = storage.loadBlockElement(Height(1));

		model::PreviousBlockContext context(*pNemesisBlockElement);
		auto pBlock = model::CreateBlock(context, Network_Identifier, signer.publicKey(), model::Transactions());
		pBlock->Timestamp = context.Timestamp + Timestamp(60000);
		SignBlock(signer, *pBlock);
		return std::move(pBlock);
	}
}}
