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

#include "ImportanceHeight.h"
#include "catapult/exceptions.h"

namespace catapult { namespace model {

	namespace {
		void CheckImportanceGrouping(Height::ValueType grouping) {
			if (0 == grouping)
				CATAPULT_THROW_INVALID_ARGUMENT_1("importance grouping must be non-zero", grouping);
		}
	}

	ImportanceHeight ConvertToImportanceHeight(Height height, Height::ValueType grouping) {
		CheckImportanceGrouping(grouping);
		Height::ValueType previousHeight = height.unwrap() - 1;
		Height::ValueType groupedHeight = (previousHeight / grouping) * grouping;
		return ImportanceHeight(groupedHeight < 1 ? 1 : groupedHeight);
	}
}}
