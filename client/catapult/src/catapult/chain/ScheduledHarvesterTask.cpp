#include "ScheduledHarvesterTask.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace chain {

	void ScheduledHarvesterTask::harvest() {
		if (m_isAnyHarvestedBlockPending || !m_harvestingAllowed())
			return;

		auto pLastBlockElement = m_lastBlockElementSupplier();
		auto pBlock = m_pHarvester->harvest(*pLastBlockElement, m_timeGenerator());
		if (!pBlock)
			return;

		CATAPULT_LOG(info) << "Successfully harvested block at " << pBlock->Height << " with signer "
				<< utils::HexFormat(pBlock->Signer);
		m_isAnyHarvestedBlockPending = true;
		m_rangeConsumer(model::BlockRange::FromEntity(std::move(pBlock)), [&isBlockPending = m_isAnyHarvestedBlockPending](auto, auto) {
			isBlockPending = false;
		});
	}
}}
