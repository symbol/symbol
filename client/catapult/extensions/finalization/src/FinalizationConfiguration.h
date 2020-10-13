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

#pragma once
#include "catapult/utils/BlockSpan.h"
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"
#include <boost/filesystem/path.hpp>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace finalization {

	/// Finalization configuration settings.
	struct FinalizationConfiguration {
	public:
		/// \c true if this node should participate in voting.
		bool EnableVoting;

		/// \c true if this node should resend votes on boot.
		/// \note When \c true, a network is able to recover from a catastrophic outage that takes all nodes offline.
		bool EnableRevoteOnBoot;

		/// Finalization size.
		uint64_t Size;

		/// Finalization threshold.
		uint64_t Threshold;

		/// Duration of a finalization step.
		utils::TimeSpan StepDuration;

		/// Duration of a finalization message in the short lived cache.
		utils::TimeSpan ShortLivedCacheMessageDuration;

		/// Maximum size of a finalization message synchronization response.
		utils::FileSize MessageSynchronizationMaxResponseSize;

		/// Maximum number of hashes to finalize per finalization point.
		uint32_t MaxHashesPerPoint;

		/// Height multiple of the last block in a prevote hash chain.
		uint16_t PrevoteBlocksMultiple;

		/// Voting key dilution.
		uint16_t VotingKeyDilution;

		/// Target duration of unfinalized blocks.
		/// \note This should be zero when `EnableVoting` is \c true.
		utils::BlockSpan UnfinalizedBlocksDuration;

		/// Number of blocks that should be treated as a group for voting set purposes.
		/// \note This is copied from BlockChainConfiguration for easy access.
		uint64_t VotingSetGrouping;

	private:
		FinalizationConfiguration() = default;

	public:
		/// Creates an uninitialized finalization configuration.
		static FinalizationConfiguration Uninitialized();

	public:
		/// Loads a finalization configuration from \a bag.
		static FinalizationConfiguration LoadFromBag(const utils::ConfigurationBag& bag);

		/// Loads a finalization configuration from \a resourcesPath.
		static FinalizationConfiguration LoadFromPath(const boost::filesystem::path& resourcesPath);
	};
}}
