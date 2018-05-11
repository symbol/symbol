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
#include "catapult/deltaset/BaseSetDelta.h"
#include <numeric>

namespace catapult { namespace cache {

	/// A mixin for calculating the deep size of mosaics.
	template<typename TSet>
	class MosaicDeepSizeMixin {
	public:
		/// Creates a mixin around \a deepSize.
		explicit MosaicDeepSizeMixin(size_t deepSize) : m_deepSize(deepSize)
		{}

	public:
		/// Gets the total number of mosaics in the cache (including versions).
		size_t deepSize() const {
			return m_deepSize;
		}

	protected:
		/// Increments the deep size.
		void incrementDeepSize() {
			++m_deepSize;
		}

		/// Decrements the deep size by \a delta.
		void decrementDeepSize(size_t delta = 1) {
			m_deepSize -= delta;
		}

	private:
		size_t m_deepSize;
	};
}}
