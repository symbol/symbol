#pragma once
#include "Options.h"

namespace catapult { namespace tools {

	/// Interface for the tools.
	class Tool {
	public:
		virtual ~Tool() {}

	public:
		/// Returns name of the tool.
		virtual std::string name() const = 0;

		/// Prepare named (\a optionsBuilder) and \a positional options of the tool.
		virtual void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional& positional) = 0;

		/// Run the tool passing \a options collected from the command line.
		virtual int run(const Options& options) = 0;
	};
}}
