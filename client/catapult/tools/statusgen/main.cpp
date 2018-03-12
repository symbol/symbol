#include "tools/ToolMain.h"
#include "catapult/model/FacilityCode.h"
#include "catapult/utils/HexFormatter.h"
#include "../../src/catapult/validators/ValidationResult.h" /* notice that validators are not in sdk */
#include <iostream>

using namespace catapult::validators;

#define DEFINE_WELL_KNOWN_RESULT(CODE) Output(ValidationResult::CODE, #CODE)

#define CUSTOM_RESULT_DEFINITION 1
#undef DEFINE_VALIDATION_RESULT

#define STR(SYMBOL) #SYMBOL
#define DEFINE_VALIDATION_RESULT(SEVERITY, FACILITY, DESCRIPTION, CODE, FLAGS) \
		Output( \
				MakeValidationResult((ResultSeverity::SEVERITY), (FacilityCode::FACILITY), CODE, (ResultFlags::FLAGS)), \
				STR(SEVERITY##_##FACILITY##_##DESCRIPTION));

namespace catapult { namespace tools { namespace statusgen {

	namespace {
		void Output(ValidationResult result, const std::string& friendlyName) {
			std::cout << "case 0x" << utils::HexFormat(result) << ": return '" << friendlyName << "';" << std::endl;
		}

		class StatusTool : public Tool {
		public:
			std::string name() const override {
				return "Status Generator";
			}

			void prepareOptions(OptionsBuilder&, OptionsPositional&) override {
			}

			int run(const Options&) override {
				DEFINE_WELL_KNOWN_RESULT(Success);
				DEFINE_WELL_KNOWN_RESULT(Neutral);
				DEFINE_WELL_KNOWN_RESULT(Failure);

				// allow this tool to reach into src and plugins and not be limited by catapult-sdk
				#include "../../plugins/coresystem/src/validators/Results.h"
				#include "../../plugins/services/hashcache/src/validators/Results.h"
				#include "../../plugins/services/signature/src/validators/Results.h"
				#include "../../plugins/txes/aggregate/src/validators/Results.h"
				#include "../../plugins/txes/lock/src/validators/Results.h"
				#include "../../plugins/txes/multisig/src/validators/Results.h"
				#include "../../plugins/txes/namespace/src/validators/Results.h"
				#include "../../plugins/txes/transfer/src/validators/Results.h"
				#include "../../src/catapult/consumers/BlockChainProcessorResults.h"
				#include "../../src/catapult/consumers/ConsumerResults.h"
				#include "../../src/catapult/extensions/Results.h"

				return 0;
			}
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::statusgen::StatusTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
