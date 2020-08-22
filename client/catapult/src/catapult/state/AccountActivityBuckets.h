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
#include "CompactArrayStack.h"
#include "catapult/model/HeightGrouping.h"
#include "catapult/functions.h"

namespace catapult { namespace state {

	/// Stack of account activity buckets.
	class AccountActivityBuckets {
	public:
		/// Temporal activity information excluding height.
		struct HeightDetachedActivityBucket {
			/// Total fees paid by account.
			Amount TotalFeesPaid;

			/// Number of times account has been used as a beneficiary.
			uint32_t BeneficiaryCount = 0;

			/// Optional user defined score component.
			uint64_t RawScore = 0;
		};

		/// Temporal activity information including height.
		struct ActivityBucket : public HeightDetachedActivityBucket {
			/// Activity start height.
			model::ImportanceHeight StartHeight;
		};

	private:
		using ActivityBucketStack = CompactArrayStack<ActivityBucket, Activity_Bucket_History_Size>;

	public:
		/// Gets the activity bucket at \a height.
		ActivityBucket get(model::ImportanceHeight height) const;

	public:
		/// Updates the bucket at \a height by passing its components to \a consumer.
		/// \note This will create a new bucket if one does not exist at \a height.
		void update(model::ImportanceHeight height, const consumer<HeightDetachedActivityBucket&>& consumer);

		/// Tries to update the bucket at \a height by passing its components to \a consumer.
		/// \note This will never create a new bucket.
		bool tryUpdate(model::ImportanceHeight height, const consumer<HeightDetachedActivityBucket&>& consumer);

		/// Pops the current bucket.
		void pop();

	public:
		/// Gets a const iterator to the first element of the underlying container.
		ActivityBucketStack::const_iterator begin() const;

		/// Gets a const iterator to the element following the last element of the underlying container.
		ActivityBucketStack::const_iterator end() const;

	private:
		bool tryUpdate(
				model::ImportanceHeight height,
				const consumer<HeightDetachedActivityBucket&>& consumer,
				bool shouldCreateNewBucket);

	private:
		ActivityBucketStack m_buckets;
	};
}}
