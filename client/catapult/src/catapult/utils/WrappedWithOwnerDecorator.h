#pragma once
#include <memory>

namespace catapult { namespace utils {

	/// Decorates a handler by additionally capturing a shared pointer to an owning object,
	/// which extends that object's lifetime.
	template<typename THandler>
	class WrappedWithOwnerDecorator {
	public:
		/// Wraps \a handler and keeps \a pOwner alive.
		template<typename TOwner>
		WrappedWithOwnerDecorator(const std::shared_ptr<TOwner>& pOwner, THandler handler)
				: m_pOwner(pOwner)
				, m_handler(std::move(handler))
		{}

	public:
		/// Forwards \a args to the wrapped handler.
		template<typename... TArgs>
		auto operator()(TArgs&& ...args) const {
			return m_handler(std::forward<TArgs>(args)...);
		}

		/// A reference to the owning pointer.
		const std::shared_ptr<const void>& owner() const {
			return m_pOwner;
		}

	protected:
		std::shared_ptr<const void> m_pOwner; // owner must be destroyed last in case handler holds raw reference(s) to it
		THandler m_handler;
	};

	/// Decorates a handler by additionally capturing a shared pointer to an owning object,
	/// which extends that object's lifetime. This decorator supports reset.
	template<typename THandler>
	class ResettableWrappedWithOwnerDecorator : public WrappedWithOwnerDecorator<THandler> {
	public:
		/// Wraps \a handler and keeps \a pOwner alive.
		template<typename TOwner>
		ResettableWrappedWithOwnerDecorator(const std::shared_ptr<TOwner>& pOwner, THandler handler)
				: WrappedWithOwnerDecorator<THandler>(pOwner, std::move(handler))
		{}

	public:
		/// Releases all resources associated with this object.
		void reset() {
			using BaseType = WrappedWithOwnerDecorator<THandler>;
			auto empty = decltype(BaseType::m_handler)();
			BaseType::m_handler.swap(empty);
			BaseType::m_pOwner.reset();
		}
	};
}}
