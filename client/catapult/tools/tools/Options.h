#pragma once
#include "catapult/types.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4505)
#include <boost/program_options.hpp>
#pragma warning(pop)
#else
#include <boost/program_options.hpp>
#endif

namespace catapult { namespace tools {

	/// Name of an option where public key is kept.
	constexpr auto Server_Public_Key_Option_Name = "key";

	/// Options builder.
	using OptionsBuilder = boost::program_options::options_description_easy_init;

	/// Definition of positional arguments.
	using OptionsPositional = boost::program_options::positional_options_description;

	/// Provides access to parsed options.
	using Options = boost::program_options::variables_map;

	/// Helper wrapper to simplify defining values.
	template<typename TValue>
	auto OptionsValue(TValue& value) {
		return boost::program_options::value<TValue>(&value);
	}

	/// Helper wrapper to simplify defining toggle values.
	inline auto OptionsSwitch() {
		return boost::program_options::bool_switch();
	}

	/// Returns the key specified via \a options.
	Key GetKeyFromOptions(const Options& options);

	/// Adds a "key" option using \a optionsBuilder, uses \a key for storage.
	void AddDefaultServerKeyOption(OptionsBuilder& optionsBuilder, std::string& key);
}}
