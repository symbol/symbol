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

#pragma once
#include <filesystem>
#include <functional>
#include <memory>

namespace catapult {
	namespace config {
		class CatapultConfiguration;
		class CatapultKeys;
	}
}

namespace catapult { namespace process {

	/// Process options.
	enum class ProcessOptions {
		/// Exit immediately after process host creation.
		Exit_After_Process_Host_Creation,

		/// Wait for termination signal before exiting.
		Exit_After_Termination_Signal
	};

	/// Prototype for creating a process host.
	/// \note Return value is a shared_ptr because unique_ptr of void is not allowed.
	using CreateProcessHost = std::function<std::shared_ptr<void> (config::CatapultConfiguration&&, const config::CatapultKeys&)>;

	/// Extracts the resources path from the command line arguments.
	/// \a argc commmand line arguments are accessible via \a argv.
	std::filesystem::path GetResourcesPath(int argc, const char** argv);

	/// Main entry point for a catapult process named \a host with default process options.
	/// \a argc commmand line arguments are accessible via \a argv.
	/// \a createProcessHost creates the process host.
	int ProcessMain(int argc, const char** argv, const std::string& host, const CreateProcessHost& createProcessHost);

	/// Main entry point for a catapult process named \a host with specified process options (\a processOptions).
	/// \a argc commmand line arguments are accessible via \a argv.
	/// \a createProcessHost creates the process host.
	int ProcessMain(
			int argc,
			const char** argv,
			const std::string& host,
			ProcessOptions processOptions,
			const CreateProcessHost& createProcessHost);
}}
