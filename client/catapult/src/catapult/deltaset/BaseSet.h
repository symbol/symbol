#pragma once
#include "BaseSetDefaultTraits.h"
#include "DeltaElements.h"
#include "catapult/exceptions.h"
#include <memory>

namespace catapult { namespace deltaset {

	namespace detail {

		/// Updates \a element in \a elements.
		template<typename TSetTraits>
		void UpdateElement(typename TSetTraits::SetType& elements, const typename TSetTraits::SetType::value_type& element) {
			auto iter = elements.find(TSetTraits::ToKey(element));
			if (elements.cend() != iter) {
				iter = elements.erase(iter);
				elements.insert(iter, element);
				return;
			}

			CATAPULT_THROW_INVALID_ARGUMENT("element not found, cannot update");
		}

		/// Applies all changes in \a deltas to \a elements.
		template<typename TSetTraits>
		void UpdateBaseSet(typename TSetTraits::SetType& elements, const DeltaElements<typename TSetTraits::SetType>& deltas) {
			if (!deltas.Added.empty())
				elements.insert(deltas.Added.cbegin(), deltas.Added.cend());

			for (auto element : deltas.Copied)
				UpdateElement<TSetTraits>(elements, element);

			for (auto element : deltas.Removed)
				elements.erase(TSetTraits::ToKey(element));
		}

		/// Policy for committing changes to a base set.
		template<typename TSetTraits>
		struct BaseSetCommitPolicy {
		private:
			using SetType = typename TSetTraits::SetType;

		public:
			static void Update(SetType& elements, const DeltaElements<SetType>& deltas) {
				UpdateBaseSet<TSetTraits>(elements, deltas);
			}
		};
	}

	template<typename TElementTraits, typename TSetTraits>
	class BaseSetDelta;

	/// A base set.
	/// \tparam TElementTraits Traits describing the type of element.
	/// \tparam TSetTraits Traits describing the underlying set.
	/// \tparam TCommitPolicy The policy for committing changes to a base set.
	///
	/// \note: 1) this class is not thread safe.
	///        2) if TSetTraits::SetType is an unordered set, the element must implement operator ==
	///        3) if MutableTypeTraits are used, the element must implement a (deep) copy
	template<
			typename TElementTraits,
			typename TSetTraits,
			typename TCommitPolicy = detail::BaseSetCommitPolicy<TSetTraits>
	>
	class BaseSet : public utils::MoveOnly {
	public:
		using ElementType = typename TElementTraits::ElementType;
		using SetType = typename TSetTraits::SetType;
		using KeyType = typename TSetTraits::KeyType;
		using FindTraits = detail::FindTraits<ElementType, TSetTraits::AllowsNativeValueModification>;
		using DeltaType = BaseSetDelta<TElementTraits, TSetTraits>;

	public:
		/// Gets a value indicating whether or not the set is empty.
		bool empty() const {
			return m_elements.empty();
		}

		/// Gets the size of this set.
		size_t size() const {
			return m_elements.size();
		}

		/// Searches for \a key in this set.
		/// Returns a pointer to the matching element if it is found or \c nullptr if it is not found.
		typename FindTraits::ConstResultType find(const KeyType& key) const {
			auto iter = m_elements.find(key);
			return m_elements.cend() != iter ? FindTraits::ToResult(TSetTraits::ToValue(*iter)) : nullptr;
		}

		/// Returns an iterator that points to the element with \a key if it is contained in this set,
		/// or cend() otherwise.
		auto findIterator(const KeyType& key) const {
			return m_elements.find(key);
		}

		/// Searches for \a key in this set.
		/// Returns \c true if it is found or \c false if it is not found.
		bool contains(const KeyType& key) const {
			return m_elements.cend() != m_elements.find(key);
		}

		/// Returns a delta based on the same original elements as this set.
		std::shared_ptr<DeltaType> rebase() {
			if (m_pWeakDelta.lock())
				CATAPULT_THROW_RUNTIME_ERROR("only a single attached delta is allowed at a time");

			auto pDelta = std::make_shared<DeltaType>(m_elements);
			m_pWeakDelta = pDelta;
			return pDelta;
		}

		/// Returns a delta based on the same original elements as this set
		/// but without the ability to commit any changes to the original set.
		std::shared_ptr<DeltaType> rebaseDetached() const {
			return std::make_shared<DeltaType>(m_elements);
		}

		/// Commits all changes in the rebased cache.
		template<typename... TArgs>
		void commit(TArgs&&... args) {
			auto pDelta = m_pWeakDelta.lock();
			if (!pDelta)
				CATAPULT_THROW_RUNTIME_ERROR("attempting to commit changes to a set without any outstanding attached deltas");

			auto deltas = pDelta->deltas();
			TCommitPolicy::Update(m_elements, deltas, std::forward<TArgs>(args)...);
			pDelta->reset();
		}

	public:
		/// Returns a const iterator to the first element of the underlying set.
		auto begin() const {
			return m_elements.cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto end() const {
			return m_elements.cend();
		}

	private:
		SetType m_elements;
		std::weak_ptr<DeltaType> m_pWeakDelta;
	};
}}
