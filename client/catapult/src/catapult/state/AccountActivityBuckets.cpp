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

#include "AccountActivityBuckets.h"

namespace catapult { namespace state {

	bool AccountActivityBuckets::empty() const {
		return std::all_of(m_buckets.begin(), m_buckets.end(), [](const auto& bucket) {
			return model::ImportanceHeight() == bucket.StartHeight;
		});
	}

	AccountActivityBuckets::ActivityBucket AccountActivityBuckets::get(model::ImportanceHeight height) const {
		auto iter = std::find_if(m_buckets.begin(), m_buckets.end(), [height](const auto& bucket) {
			return bucket.StartHeight == height;
		});

		return m_buckets.end() == iter ? ActivityBucket() : *iter;
	}

	void AccountActivityBuckets::update(model::ImportanceHeight height, const consumer<HeightDetachedActivityBucket&>& consumer) {
		tryUpdate(height, consumer, true);
	}

	bool AccountActivityBuckets::tryUpdate(model::ImportanceHeight height, const consumer<HeightDetachedActivityBucket&>& consumer) {
		return tryUpdate(height, consumer, false);
	}

	void AccountActivityBuckets::push() {
		m_buckets.push(ActivityBucket());
	}

	void AccountActivityBuckets::pop() {
		m_buckets.pop();
	}

	AccountActivityBuckets::ActivityBucketStack::const_iterator AccountActivityBuckets::begin() const {
		return m_buckets.begin();
	}

	AccountActivityBuckets::ActivityBucketStack::const_iterator AccountActivityBuckets::end() const {
		return m_buckets.end();
	}

	bool AccountActivityBuckets::tryUpdate(
			model::ImportanceHeight height,
			const consumer<HeightDetachedActivityBucket&>& consumer,
			bool shouldCreateNewBucket) {
		auto lastHeight = m_buckets.begin()->StartHeight;
		if (model::ImportanceHeight() != lastHeight && lastHeight > height) {
			std::ostringstream out;
			out << "older buckets cannot be updated (last = " << lastHeight << ", new = " << height << ")";
			CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
		}

		if (lastHeight < height) {
			if (!shouldCreateNewBucket)
				return false;

			auto bucket = ActivityBucket();
			bucket.StartHeight = height;
			m_buckets.push(bucket);
		}

		auto& bucket = m_buckets.peek();
		consumer(bucket);
		return true;
	}
}}
