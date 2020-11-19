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
#include "SynchronizationFilters.h"
#include "filter_constants.h"
#include <vector>

namespace catapult { namespace timesync { namespace filters {

	/// Aggregate synchronization filter.
	class AggregateSynchronizationFilter {
	public:
		/// Creates an aggregate synchronization filter around \a filters.
		explicit AggregateSynchronizationFilter(const std::vector<SynchronizationFilter>& filters);

	public:
		/// Gets the number of filters.
		size_t size() const;

	public:
		/// Filters \a samples according to \a nodeAge.
		void operator()(TimeSynchronizationSamples& samples, NodeAge nodeAge);

	private:
		std::vector<SynchronizationFilter> m_filters;
	};
}}}
