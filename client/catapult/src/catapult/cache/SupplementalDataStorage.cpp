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
