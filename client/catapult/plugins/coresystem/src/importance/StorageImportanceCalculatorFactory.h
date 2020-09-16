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
#include "ImportanceCalculator.h"
#include "catapult/config/CatapultDataDirectory.h"

namespace catapult { namespace importance {

	/// Factory for creating importance calculators that persist importance information to disk for long term storage.
	class StorageImportanceCalculatorFactory {
	public:
		/// Creates a factory around \a directory.
		explicit StorageImportanceCalculatorFactory(const config::CatapultDirectory& directory);

	public:
		/// Decorates \a pCalculator by writing its results to disk.
		std::unique_ptr<ImportanceCalculator> createWriteCalculator(std::unique_ptr<ImportanceCalculator>&& pCalculator) const;

		/// Decorates \a pCalculator by restoring older importances from disk.
		std::unique_ptr<ImportanceCalculator> createReadCalculator(std::unique_ptr<ImportanceCalculator>&& pCalculator) const;

	private:
		config::CatapultDirectory m_directory;
	};
}}
