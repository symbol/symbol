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
#include "ImportanceCalculator.h"
#include "catapult/config/CatapultDataDirectory.h"

namespace catatpult { namespace model { struct BlockChainConfiguration; } }

namespace catapult { namespace importance {

	/// Factory for creating importance calculators that persist importance information to disk for long term storage.
	class StorageImportanceCalculatorFactory {
	public:
		/// Creates a factory around \a config.
		explicit StorageImportanceCalculatorFactory(const model::BlockChainConfiguration& config);

	public:
		/// Decorates \a pCalculator by writing its results to \a directory.
		std::unique_ptr<ImportanceCalculator> createWriteCalculator(
				std::unique_ptr<ImportanceCalculator>&& pCalculator,
				const config::CatapultDirectory& directory) const;

		/// Decorates \a pCalculator by restoring older importances from \a directory.
		std::unique_ptr<ImportanceCalculator> createReadCalculator(
				std::unique_ptr<ImportanceCalculator>&& pCalculator,
				const config::CatapultDirectory& directory) const;

	private:
		const model::BlockChainConfiguration& m_config;
	};
}}
