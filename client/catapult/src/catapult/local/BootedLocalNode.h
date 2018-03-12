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
		/// Shutdowns the local node.
		virtual void shutdown() = 0;
	};
}}
