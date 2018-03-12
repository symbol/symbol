#pragma once
#include "catapult/model/ImportanceHeight.h"
#include "catapult/types.h"
#include <memory>

namespace catapult {
	namespace cache { class AccountStateCacheDelta; }
	namespace model { struct BlockChainConfiguration; }
}

namespace catapult { namespace observers {

	/// Base class for all importance calculators.
	class ImportanceCalculator {
	public:
		virtual ~ImportanceCalculator() {}

	public:
		/// Recalculates importances for all accounts in \a cache at \a importanceHeight that are eligible for
		/// harvesting.
		virtual void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const = 0;
	};

	/// Creates an importance calculator for the block chain described by \a config.
	std::unique_ptr<ImportanceCalculator> CreateImportanceCalculator(const model::BlockChainConfiguration& config);

	/// Creates a restore importance calculator.
	std::unique_ptr<ImportanceCalculator> CreateRestoreImportanceCalculator();
}}
