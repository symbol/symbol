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

#pragma once
#include "catapult/model/HeightGrouping.h"
#include "catapult/types.h"
#include <memory>

namespace catapult {
	namespace cache { class AccountStateCacheDelta; }
	namespace model { struct BlockChainConfiguration; }
}

namespace catapult { namespace importance {

	/// Base class for all importance calculators.
	class ImportanceCalculator {
	public:
		virtual ~ImportanceCalculator() = default;

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
