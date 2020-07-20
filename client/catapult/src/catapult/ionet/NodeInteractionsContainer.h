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
#include "catapult/utils/TimeSpan.h"
#include "catapult/functions.h"
#include <list>

namespace catapult { namespace ionet {

	/// Node interactions.
	struct NodeInteractions {
	public:
		/// Default constructor for node interactions.
		NodeInteractions() : NodeInteractions(0, 0)
		{}

		/// Constructs node interactions around \a numSuccesses and \a numFailures.
		NodeInteractions(uint32_t numSuccesses, uint32_t numFailures)
				: NumSuccesses(numSuccesses)
				, NumFailures(numFailures)
		{}

	public:
		/// Number of successful interactions.
		uint32_t NumSuccesses;

		/// Number of failed interactions.
		uint32_t NumFailures;
	};

	/// Node interactions container.
	class NodeInteractionsContainer {
	private:
		struct NodeInteractionsBucket {
		public:
			/// Constructs a bucket around \a timestamp.
			explicit NodeInteractionsBucket(Timestamp timestamp)
					: CreationTime(timestamp)
					, NumSuccesses(0)
					, NumFailures(0)
			{}

		public:
			/// Time at which the bucket was created.
			Timestamp CreationTime;

			/// Number of successful interactions.
			uint32_t NumSuccesses;

			/// Number of failed interactions.
			uint32_t NumFailures;
		};

	public:
		/// Maximum duration of an interaction bucket.
		static utils::TimeSpan BucketDuration();

		/// Maximum duration of an interaction.
		static utils::TimeSpan InteractionDuration();

	public:
		/// Gets the node interactions at \a timestamp.
		NodeInteractions interactions(Timestamp timestamp) const;

	public:
		/// Increments successful interactions at \a timestamp.
		void incrementSuccesses(Timestamp timestamp);

		/// Increments failed interactions at \a timestamp.
		void incrementFailures(Timestamp timestamp);

		/// Prunes buckets at \a timestamp.
		void pruneBuckets(Timestamp timestamp);

	private:
		bool shouldCreateNewBucket(Timestamp timestamp) const;

		void addInteraction(Timestamp timestamp, const consumer<NodeInteractionsBucket&>& consumer);

	private:
		std::list<NodeInteractionsBucket> m_buckets;
	};
}}
