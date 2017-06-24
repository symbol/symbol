#pragma once
#include "Harvester.h"
#include "catapult/disruptor/DisruptorTypes.h"
#include "catapult/model/Elements.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/model/RangeTypes.h"
#include <functional>

namespace catapult { namespace chain {

	/// Options for the harvesting task.
	struct ScheduledHarvesterTaskOptions {
		using HarvestingPredicate = std::function<bool ()>;
		using LastBlockElementSupplierFunc = std::function<std::shared_ptr<const model::BlockElement> ()>;
		using TimeGeneratorFunc = std::function<Timestamp ()>;
		using RangeConsumerFunc = std::function<void (model::BlockRange&&, const disruptor::ProcessingCompleteFunc& processingComplete)>;

		/// Indicates if harvesting is allowed.
		HarvestingPredicate HarvestingAllowed;

		/// Supplies information about the last block of the chain.
		LastBlockElementSupplierFunc LastBlockElementSupplier;

		/// Supplies the current network time.
		TimeGeneratorFunc TimeGenerator;

		/// Consumes a range consisting of the harvested block, usually delivers it to the disruptor queue.
		RangeConsumerFunc RangeConsumer;
	};

	/// Class that lets a harvester create a block and supplies the block to a consumer.
	class ScheduledHarvesterTask {
	public:
		using TaskOptions = ScheduledHarvesterTaskOptions;

	public:
		/// Creates a scheduled harvesting task around \a options and a \a pHarvester.
		explicit ScheduledHarvesterTask(const ScheduledHarvesterTaskOptions& options, std::unique_ptr<Harvester>&& pHarvester)
				: m_harvestingAllowed(options.HarvestingAllowed)
				, m_lastBlockElementSupplier(options.LastBlockElementSupplier)
				, m_timeGenerator(options.TimeGenerator)
				, m_rangeConsumer(options.RangeConsumer)
				, m_pHarvester(std::move(pHarvester))
				, m_isAnyHarvestedBlockPending(false)
		{}

	public:
		/// Triggers the harvesting process and in case of successfull block creation
		/// supplies the block to the consumer.
		void harvest();

	private:
		const TaskOptions::HarvestingPredicate m_harvestingAllowed;
		const TaskOptions::LastBlockElementSupplierFunc m_lastBlockElementSupplier;
		const TaskOptions::TimeGeneratorFunc m_timeGenerator;
		const TaskOptions::RangeConsumerFunc m_rangeConsumer;
		std::unique_ptr<Harvester> m_pHarvester;

		std::atomic_bool m_isAnyHarvestedBlockPending;
	};
}}
