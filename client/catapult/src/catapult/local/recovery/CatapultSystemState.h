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
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/consumers/BlockChainSyncHandlers.h"

namespace catapult { namespace local {

	/// Catapult system state.
	class CatapultSystemState {
	public:
		/// Creates state around \a dataDirectory.
		explicit CatapultSystemState(const config::CatapultDataDirectory& dataDirectory);

	public:
		/// \c true if broker should be recovered.
		bool shouldRecoverBroker() const;

		/// \c true if server should be recovered.
		bool shouldRecoverServer() const;

		/// Last server commit operation step that succeeded.
		consumers::CommitOperationStep commitStep() const;

	public:
		/// Resets system state to indicate no crash.
		void reset();

	private:
		std::string qualifyRootFile(const std::string& filename) const;

	private:
		config::CatapultDataDirectory m_dataDirectory;
	};
}}
