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
#include "catapult/utils/FileSize.h"

namespace catapult { namespace disruptor {

	/// Consumer dispatcher options.
	struct ConsumerDispatcherOptions {
	public:
		/// Creates options around \a dispatcherName and \a disruptorSlotCount.
		constexpr ConsumerDispatcherOptions(const char* dispatcherName, size_t disruptorSlotCount)
				: DispatcherName(dispatcherName)
				, DisruptorSlotCount(disruptorSlotCount)
				, DisruptorMaxMemorySize(utils::FileSize::FromMegabytes(1024))
				, ElementTraceInterval(1)
				, ShouldThrowWhenFull(true)
		{}

	public:
		/// Name of the dispatcher.
		const char* DispatcherName;

		/// Number of slots in the disruptor circular buffer.
		size_t DisruptorSlotCount;

		/// Maximum memory of all elements in the disruptor circular buffer.
		utils::FileSize DisruptorMaxMemorySize;

		/// Multiple of elements at which an element should be traced through queue and completion.
		size_t ElementTraceInterval;

		/// \c true if the dispatcher should throw when full, \c false if it should return an error.
		bool ShouldThrowWhenFull;
	};
}}
