#pragma once
#include <utility>

namespace catapult { namespace cache {

	/// Decorates a view and supplies a read-only view.
	template<typename TView>
	class ReadOnlyViewSupplier : public TView {
	public:
		/// Creates a read-only view supplier around \a args.
		template<typename... TArgs>
		ReadOnlyViewSupplier(TArgs&&... args)
				: TView(std::forward<TArgs>(args)...)
				, m_readOnlyView(*this)
		{}

		/// Move constructs a read-only view supplier from \a rhs.
		/// \note Default move constructor will not work because pointer in m_readOnlyView needs to be updated.
		ReadOnlyViewSupplier(ReadOnlyViewSupplier&& rhs)
				: TView(std::move(rhs))
				, m_readOnlyView(*this)
		{}

	public:
		/// Returns a read-only view of this view.
		const typename TView::ReadOnlyView& asReadOnly() const {
			return m_readOnlyView;
		}

	private:
		typename TView::ReadOnlyView m_readOnlyView;
	};
}}
