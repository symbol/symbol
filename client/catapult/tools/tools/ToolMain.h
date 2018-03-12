#pragma once
#include "Tool.h"

namespace catapult { namespace tools {

	/// Bootstraps and executes the \a tool.
	/// \a argc and \a argv are arguments passed to main.
	int ToolMain(int argc, const char** argv, Tool& tool);
}}
