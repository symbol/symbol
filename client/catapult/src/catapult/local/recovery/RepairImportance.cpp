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

#include "RepairImportance.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/FilesystemUtils.h"

namespace catapult { namespace local {

	void RepairImportance(const config::CatapultDataDirectory& dataDirectory, consumers::CommitOperationStep commitStep) {
		auto importanceDirectory = dataDirectory.dir("importance");
		auto importanceWipDirectory = importanceDirectory.dir("wip");
		if (consumers::CommitOperationStep::State_Written != commitStep) {
			// importance/wip files should be purged if state hasn't been fully written
			CATAPULT_LOG(debug) << " - purging " << importanceWipDirectory.str();
			io::PurgeDirectory(importanceWipDirectory.str());
			return;
		} else {
			// otherwise, copy importance/wip files to commit them
			CATAPULT_LOG(debug) << " - committing " << importanceWipDirectory.str();
			io::MoveAllFiles(importanceWipDirectory.str(), importanceDirectory.str());
		}
	}
}}
