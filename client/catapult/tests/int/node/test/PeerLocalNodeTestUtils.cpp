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

#include "PeerLocalNodeTestUtils.h"
#include "LocalNodeRequestTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	PeerLocalNodeTestContext::PeerLocalNodeTestContext(NodeFlag nodeFlag)
			: m_context(nodeFlag | NodeFlag::With_Partner, { CreateLocalPartnerNode() })
	{}

	local::BootedLocalNode& PeerLocalNodeTestContext::localNode() const {
		return m_context.localNode();
	}

	PeerLocalNodeStats PeerLocalNodeTestContext::stats() const {
		return m_context.stats();
	}

	Height PeerLocalNodeTestContext::height() const {
		ExternalSourceConnection connection;
		return GetLocalNodeHeightViaApi(connection);
	}

	void PeerLocalNodeTestContext::waitForHeight(Height height) const {
		ExternalSourceConnection connection;
		WaitForLocalNodeHeight(connection, height);
	}

	config::LocalNodeConfiguration PeerLocalNodeTestContext::prepareFreshDataDirectory(const std::string& directory) const {
		return m_context.prepareFreshDataDirectory(directory);
	}

	void PeerLocalNodeTestContext::assertSingleReaderConnection() const {
		AssertSingleReaderConnection(stats());
	}

	void PeerLocalNodeTestContext::AssertSingleReaderConnection(const PeerLocalNodeStats& stats) {
		// Assert: the external reader connection is still active
		EXPECT_EQ(1u, stats.NumActiveReaders);
		EXPECT_EQ(1u, stats.NumActiveWriters);
		EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
	}
}}
