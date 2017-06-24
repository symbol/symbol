#pragma once

namespace catapult { namespace utils {

	/// A class that can neither be copied nor moved.
	class NonCopyable {
	public:
		/// Default constructor.
		constexpr NonCopyable() = default;

		/// Default destructor.
		~NonCopyable() = default;

	public:
		/// Disabled copy constructor.
		NonCopyable(const NonCopyable&) = delete;

		/// Disabled assignment operator.
		NonCopyable& operator=(const NonCopyable&) = delete;
	};

	/// A class that can be moved but not copied.
	class MoveOnly {
	public:
		/// Default constructor.
		constexpr MoveOnly() = default;

		/// Default destructor.
		~MoveOnly() = default;

	public:
		/// Disabled copy constructor.
		MoveOnly(const NonCopyable&) = delete;

		/// Default move constructor.
		MoveOnly(MoveOnly&&) = default;

		/// Disabled assignment operator.
		MoveOnly& operator=(const MoveOnly&) = delete;

		/// Default move assignment operator.
		MoveOnly& operator=(MoveOnly&&) = default;
	};
}}
