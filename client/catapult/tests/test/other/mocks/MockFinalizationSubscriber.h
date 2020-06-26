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
#include "catapult/subscribers/FinalizationSubscriber.h"
#include "tests/test/nodeps/ParamsCapture.h"

namespace catapult { namespace mocks {

	/// Finalization subscriber finalized block params.
	struct FinalizationSubscriberFinalizedBlockParams {
	public:
		/// Creates params around \a height, \a hash and \a point.
		FinalizationSubscriberFinalizedBlockParams(catapult::Height height, const Hash256& hash, FinalizationPoint point)
				: Height(height)
				, Hash(hash)
				, Point(point)
		{}

	public:
		/// Block height.
		const catapult::Height Height;

		/// Block hash.
		const Hash256 Hash;

		/// Finalization point.
		const FinalizationPoint Point;
	};

	/// Mock finalization subscriber implementation.
	class MockFinalizationSubscriber : public subscribers::FinalizationSubscriber {
	public:
		/// Gets the params passed to notifyFinalizedBlock.
		const auto& finalizedBlockParams() const {
			return m_finalizedBlockParams;
		}

	public:
		void notifyFinalizedBlock(Height height, const Hash256& hash, FinalizationPoint point) override {
			m_finalizedBlockParams.push(height, hash, point);
		}

	private:
		test::ParamsCapture<FinalizationSubscriberFinalizedBlockParams> m_finalizedBlockParams;
	};
}}
