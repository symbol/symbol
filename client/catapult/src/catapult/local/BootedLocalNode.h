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
#include "catapult/utils/DiagnosticCounter.h"
#include "catapult/utils/NonCopyable.h"
#include <vector>

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace ionet { class NodeContainerView; }
	namespace model { class ChainScore; }
}

namespace catapult { namespace local {

	/// A local node counter value.
	/// \note This is a constant value unlike utils::DiagnosticCounter.
	class LocalNodeCounterValue {
	public:
		/// Creates a counter value from \a counter.
		explicit LocalNodeCounterValue(const utils::DiagnosticCounter& counter)
				: m_id(counter.id())
				, m_value(counter.value())
		{}

	public:
		/// Gets the counter id.
		const utils::DiagnosticCounterId& id() const {
			return m_id;
		}

		/// Gets the counter value.
		uint64_t value() const {
			return m_value;
		}

	private:
		utils::DiagnosticCounterId m_id;
		uint64_t m_value;
	};

	/// A container of local node counter values.
	using LocalNodeCounterValues = std::vector<LocalNodeCounterValue>;

	/// Represents a booted local node.
	class BootedLocalNode : public utils::NonCopyable {
	public:
		virtual ~BootedLocalNode() {}

	public:
		/// Gets the current cache.
		virtual const cache::CatapultCache& cache() const = 0;

		/// Gets the current chain score.
		virtual model::ChainScore score() const = 0;

		/// Gets the current node counters.
		virtual LocalNodeCounterValues counters() const = 0;

		/// Gets the current nodes.
		virtual ionet::NodeContainerView nodes() const = 0;

	public:
		/// Shuts down the local node.
		virtual void shutdown() = 0;
	};
}}
