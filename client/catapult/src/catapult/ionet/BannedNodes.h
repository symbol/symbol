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
#include "catapult/model/NodeIdentity.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/functions.h"
#include <unordered_map>

namespace catapult { namespace ionet {

	/// Ban settings.
	struct BanSettings {
		/// Default duration for banning.
		utils::TimeSpan DefaultBanDuration;

		/// Maximum duration for banning.
		utils::TimeSpan MaxBanDuration;

		/// Duration to keep account in container after the ban expired.
		utils::TimeSpan KeepAliveDuration;

		/// Maximum number of banned nodes.
		uint32_t MaxBannedNodes;
	};

	/// Container for banned nodes.
	class BannedNodes {
	private:
		struct BannedNode {
			model::NodeIdentity BannedNodeIdentity;
			Timestamp BanStart;
			utils::TimeSpan BanDuration;
			uint32_t Reason;
		};

		using BannedNodesContainer = model::NodeIdentityMap<BannedNode>;

	public:
		/// Creates banned nodes container around \a banSettings, \a timeSupplier and \a equalityStrategy.
		BannedNodes(
				const BanSettings& banSettings,
				const supplier<Timestamp>& timeSupplier,
				model::NodeIdentityEqualityStrategy equalityStrategy);

	public:
		/// Gets the number of entries excluding expired bans.
		size_t size() const;

		/// Gets the number of entries including expired bans.
		size_t deepSize() const;

		/// Returns \c true if \a nodeIdentity is banned.
		bool isBanned(const model::NodeIdentity& nodeIdentity) const;

	public:
		/// Adds \a nodeIdentity to the container due to \a reason.
		void add(const model::NodeIdentity& nodeIdentity, uint32_t reason);

		/// Removes banned nodes whose ban has expired.
		void prune();

	private:
		void removeEntryThatExpiresNext();

	private:
		static bool IsBanned(const BannedNode& bannedNode, Timestamp timestamp);

	private:
		BanSettings m_banSettings;
		supplier<Timestamp> m_timeSupplier;
		BannedNodesContainer m_bannedNodes;
	};
}}
