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

#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/local/recovery/RecoveryOrchestrator.h"
#include "catapult/process/ProcessMain.h"

namespace {
	constexpr auto Process_Name = "recovery";
}

int main(int argc, const char** argv) {
	using namespace catapult;

	auto processOptions = process::ProcessOptions::Exit_After_Process_Host_Creation;
	return process::ProcessMain(argc, argv, Process_Name, processOptions, [argc, argv](auto&& config, const auto&) {
		// create bootstrapper
		auto resourcesPath = process::GetResourcesPath(argc, argv).generic_string();
		auto disposition = extensions::ProcessDisposition::Recovery;
		auto pBootstrapper = std::make_unique<extensions::ProcessBootstrapper>(config, resourcesPath, disposition, Process_Name);

		// register extension(s)
		pBootstrapper->loadExtensions();

		// create the local node
		return local::CreateRecoveryOrchestrator(std::move(pBootstrapper));
	});
}
