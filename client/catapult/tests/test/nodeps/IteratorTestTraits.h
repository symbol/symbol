#pragma once

namespace catapult { namespace test {

	/// Traits for (mutable) begin() / end().
	struct BeginEndTraits {
		template<typename T>
		static auto begin(T& container) { return container.begin(); }

		template<typename T>
		static auto end(T& container) { return container.end(); }
	};

	/// Traits for (const) begin() / end().
	struct BeginEndConstTraits {
		template<typename T>
		static auto begin(const T& container) { return container.begin(); }

		template<typename T>
		static auto end(const T& container) { return container.end(); }
	};

	/// Traits for (const) cbegin() / cend().
	struct CBeginCEndTraits {
		template<typename T>
		static auto begin(T& container) { return container.cbegin(); }

		template<typename T>
		static auto end(T& container) { return container.cend(); }
	};
}}
