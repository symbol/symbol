#pragma once
#include <vector>

namespace catapult { namespace test {

	/// Base class for mocks that capture parameters.
	template<typename TParams>
	class ParamsCapture {
	public:
		virtual ~ParamsCapture() {}

	public:
		/// The captured parameters.
		const std::vector<TParams>& params() const {
			return m_params;
		}

	public:
		/// Clears the captured parameters.
		void clear() {
			m_params.clear();
		}

	public:
		/// Captures \a args.
		template<typename... TArgs>
		void push(TArgs&&... args) {
			m_params.push_back(TParams(std::forward<TArgs>(args)...));
		}

	private:
		std::vector<TParams> m_params;
	};
}}
