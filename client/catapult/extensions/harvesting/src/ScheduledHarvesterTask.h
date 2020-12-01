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
#include "Harvester.h"
#include "catapult/chain/ChainFunctions.h"
#include "catapult/disruptor/DisruptorTypes.h"
#include "catapult/model/Elements.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/functions.h"

namespace catapult { namespace harvesting {

	/// Options for the harvesting task.
	struct ScheduledHarvesterTaskOptions {
		/// Indicates if harvesting is allowed.
		predicate<> HarvestingAllowed;

		/// Supplies information about the last block of the chain.
		supplier<std::shared_ptr<const model::BlockElement>> LastBlockElementSupplier;

		/// Supplies the current network time.
		chain::TimeSupplier TimeSupplier;

		/// Consumes a range consisting of the harvested block, usually delivers it to the disruptor queue.
		consumer<model::BlockRange&&, const disruptor::ProcessingCompleteFunc&> RangeConsumer;
	};

	/// Class that lets a harvester create a block and supplies the block to a consumer.
	class ScheduledHarvesterTask {
	public:
		using TaskOptions = ScheduledHarvesterTaskOptions;

	public:
		/// Creates a scheduled harvesting task around \a options and \a pHarvester.
		ScheduledHarvesterTask(const ScheduledHarvesterTaskOptions& options, std::unique_ptr<Harvester>&& pHarvester);

	public:
		/// Triggers the harvesting process and in case of successfull block creation
		/// supplies the block to the consumer.
		void harvest();

	private:
		const decltype(TaskOptions::HarvestingAllowed) m_harvestingAllowed;
		const decltype(TaskOptions::LastBlockElementSupplier) m_lastBlockElementSupplier;
		const decltype(TaskOptions::TimeSupplier) m_timeSupplier;
		const decltype(TaskOptions::RangeConsumer) m_rangeConsumer;
		std::unique_ptr<Harvester> m_pHarvester;

		std::shared_ptr<std::atomic_bool> m_pIsAnyHarvestedBlockPending;
	};
}}
