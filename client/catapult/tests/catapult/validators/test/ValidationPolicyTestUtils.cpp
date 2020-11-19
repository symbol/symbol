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

#include "ValidationPolicyTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"

namespace catapult { namespace test {

	namespace {
		auto GenerateBlockWithTransactions(size_t count) {
			auto transactions = GenerateRandomTransactions(count);
			size_t i = 0;
			for (const auto& pTransaction : transactions)
				pTransaction->Deadline = Timestamp(i++); // use deadline as a unique entity id

			return test::GenerateBlockWithTransactions(transactions);
		}
	}

	EntityInfoContainerWrapper::EntityInfoContainerWrapper(size_t count)
			: m_pBlock(GenerateBlockWithTransactions(count))
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
