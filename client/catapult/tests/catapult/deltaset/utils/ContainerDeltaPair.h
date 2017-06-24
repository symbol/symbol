#pragma once
#include <memory>

namespace catapult { namespace test {

	/// Pair composed of a container and a container delta.
	template<typename TTraits>
	class ContainerDeltaPair {
	public:
		using Type = typename TTraits::Type;
		using DeltaType = typename TTraits::DeltaType;

	public:
		/// Wraps \a pContainer and \a pContainerDelta.
		explicit ContainerDeltaPair(
				const std::shared_ptr<Type>& pContainer,
				const std::shared_ptr<DeltaType>& pContainerDelta)
				: m_pContainer(pContainer)
				, m_pContainerDelta(pContainerDelta)
		{}

	public:
		/// Returns a pointer to the underlying delta.
		auto operator->() const {
			return m_pContainerDelta.get();
		}

		/// Returns a reference to the underlying delta.
		auto& operator*() const {
			return *m_pContainerDelta;
		}

	public:
		/// Gets the size of the original (base) container.
		size_t originalSize() const {
			return m_pContainer->size();
		}

	public:
		/// Commits delta changes to the underlying container.
		void commit() {
			m_pContainer->commit();
		}

	private:
		std::shared_ptr<Type> m_pContainer;
		std::shared_ptr<DeltaType> m_pContainerDelta;
	};
}}
