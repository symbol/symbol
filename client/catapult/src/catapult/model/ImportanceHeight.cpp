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
