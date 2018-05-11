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

#include "SupplementalDataStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace cache {

	void SaveSupplementalData(const SupplementalData& supplementalData, Height chainHeight, io::OutputStream& output) {
		io::Write(output, supplementalData.State.LastRecalculationHeight);

		auto scoreArray = supplementalData.ChainScore.toArray();
		io::Write64(output, scoreArray[0]);
		io::Write64(output, scoreArray[1]);

		io::Write(output, chainHeight);

		CATAPULT_LOG(debug)
				<< "wrote last recalculation height " << supplementalData.State.LastRecalculationHeight
				<< " (score = [" << scoreArray[0] << ", " << scoreArray[1] << "]"
				<< ", height = " << chainHeight << ")";

		output.flush();
	}

	void LoadSupplementalData(io::InputStream& input, SupplementalData& supplementalData, Height& chainHeight) {
		io::Read(input, supplementalData.State.LastRecalculationHeight);
		auto scoreHigh = io::Read64(input);
		auto scoreLow = io::Read64(input);
		supplementalData.ChainScore = model::ChainScore(scoreHigh, scoreLow);
		io::Read(input, chainHeight);

		CATAPULT_LOG(debug)
				<< "read last recalculation height " << supplementalData.State.LastRecalculationHeight
				<< " (score = [" << scoreHigh << ", " << scoreLow << "]"
				<< ", height = " << chainHeight << ")";
	}
}}
