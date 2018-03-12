#include "ValidationPolicyTestUtils.h"
#include "catapult/validators/AggregateEntityValidator.h"
#include "tests/test/core/BlockTestUtils.h"

namespace catapult { namespace test {

	namespace {
		auto GenerateRandomBlockWithTransactions(size_t count) {
			auto transactions = GenerateRandomTransactions(count);
			size_t i = 0;
			for (const auto& pTransaction : transactions)
				pTransaction->Deadline = Timestamp(i++); // use deadline as a unique entity id

			return test::GenerateRandomBlockWithTransactions(transactions);
		}
	}

	EntityInfoContainerWrapper::EntityInfoContainerWrapper(size_t count)
			: m_pBlock(GenerateRandomBlockWithTransactions(count))
			, m_container(m_pBlock->Transactions())
			, m_hashes(count) {
		for (auto i = 0u; i < count; ++i)
			m_hashes[i] = Hash256{ { static_cast<uint8_t>(i) } };
	}

	model::WeakEntityInfos EntityInfoContainerWrapper::toVector() const {
		model::WeakEntityInfos entities;

		auto i = 0u;
		for (const auto& entity : m_container)
			entities.push_back(model::WeakEntityInfo(entity, m_hashes[i++]));

		return entities;
	}

	EntityInfoContainerWrapper CreateEntityInfos(size_t count) {
		return EntityInfoContainerWrapper(count);
	}
}}
