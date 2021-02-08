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
#include "catapult/model/ChainScore.h"
#include "catapult/observers/ObserverTypes.h"
#include <functional>

namespace catapult {
	namespace extensions { struct LocalNodeStateRef; }
	namespace model {
		struct Block;
		struct BlockChainConfiguration;
		struct BlockElement;
	}
	namespace plugins { class PluginManager; }
	namespace subscribers { struct StateChangeInfo; }
}

namespace catapult { namespace local {

	/// Notification observer factory.
	using NotificationObserverFactory = supplier<std::unique_ptr<const observers::NotificationObserver>>;

	/// Block dependent notification observer factory.
	using BlockDependentNotificationObserverFactory =
		std::function<std::unique_ptr<const observers::NotificationObserver> (const model::Block&)>;

	/// Creates a block dependent notification observer factory that calculates an inflection point from \a lastBlock and \a config.
	/// Prior to the inflection point, an observer created by \a permanentObserverFactory is returned.
	/// At and after the inflection point, an observer created by \a transientObserverFactory is returned.
	BlockDependentNotificationObserverFactory CreateBlockDependentNotificationObserverFactory(
			const model::Block& lastBlock,
			const model::BlockChainConfiguration& config,
			const NotificationObserverFactory& transientObserverFactory,
			const NotificationObserverFactory& permanentObserverFactory);

	/// Information about each loaded block.
	struct LoadedBlockStatus {
		/// Loaded block element.
		const model::BlockElement& BlockElement;

		/// Chain score after applying the block.
		const model::ChainScore& ChainScore;

		/// State change information.
		const subscribers::StateChangeInfo& StateChangeInfo;
	};

	/// Loads a block chain from storage using the supplied observer factory (\a observerFactory) and plugin manager (\a pluginManager)
	/// and updating \a stateRef starting with the block at \a startHeight.
	/// Each loaded block and supporting information is passed to \a statusConsumer.
	model::ChainScore LoadBlockChain(
			const BlockDependentNotificationObserverFactory& observerFactory,
			const plugins::PluginManager& pluginManager,
			const extensions::LocalNodeStateRef& stateRef,
			Height startHeight,
			const consumer<LoadedBlockStatus&&>& statusConsumer = consumer<LoadedBlockStatus>());
}}
