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

#include "ScheduledHarvesterTask.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace harvesting {

	ScheduledHarvesterTask::ScheduledHarvesterTask(const ScheduledHarvesterTaskOptions& options, std::unique_ptr<Harvester>&& pHarvester)
			: m_harvestingAllowed(options.HarvestingAllowed)
			, m_lastBlockElementSupplier(options.LastBlockElementSupplier)
			, m_timeSupplier(options.TimeSupplier)
			, m_rangeConsumer(options.RangeConsumer)
			, m_pHarvester(std::move(pHarvester))
			, m_pIsAnyHarvestedBlockPending(std::make_shared<std::atomic_bool>(false))
	{}

	void ScheduledHarvesterTask::harvest() {
		if (*m_pIsAnyHarvestedBlockPending || !m_harvestingAllowed())
			return;

		auto pLastBlockElement = m_lastBlockElementSupplier();
		auto pBlock = m_pHarvester->harvest(*pLastBlockElement, m_timeSupplier());
		if (!pBlock)
			return;

		CATAPULT_LOG(info) << "successfully harvested block at " << pBlock->Height << " with signer " << pBlock->SignerPublicKey;
		*m_pIsAnyHarvestedBlockPending = true;

		// flag needs to be captured as shared_ptr in order to avoid race condition when harvesting service
		// is shutdown before dispatcher service and block completes processing in interim period
		m_rangeConsumer(model::BlockRange::FromEntity(std::move(pBlock)), [pIsAnyBlockPending = m_pIsAnyHarvestedBlockPending](
				auto,
				auto) {
			*pIsAnyBlockPending = false;
		});
	}
}}
