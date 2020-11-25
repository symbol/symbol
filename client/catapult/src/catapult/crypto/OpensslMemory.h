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
#include <array>
#include <atomic>
#include <memory>
#include <vector>

namespace catapult { namespace crypto {

	/// Alignment helper.
	template<size_t Alignment>
	struct AlignmentHelper {
		/// Align \a size up.
		static constexpr size_t Size(size_t size) {
			return ((size + Alignment - 1) / Alignment) * Alignment;
		}

		/// Align \a pointer up.
		static uint8_t* Pointer(uint8_t* pointer) {
			void* voidPointer = reinterpret_cast<void*>(pointer);
			size_t space = 2 * Alignment;
			return reinterpret_cast<uint8_t*>(std::align(Alignment, 0, voidPointer, space));
		}
	};

	/// Fixed size pool traits.
	template<size_t UnalignedElementSize, size_t ElementSize, size_t NumElements>
	struct FixedSizePoolTraits {
		static constexpr auto Unaligned_Element_Size = UnalignedElementSize;
		static constexpr auto Element_Size = ElementSize;
		static constexpr auto Count = NumElements;
	};

	/// Fixed size pool.
	template<typename TPoolTraits>
	class FixedSizePool : public TPoolTraits {
	public:
		static constexpr auto Size = TPoolTraits::Element_Size * TPoolTraits::Count;

	public:
		/// Creates a pool around \a pBuffer. Pointed memory must be large enough to hold \c Count * \c Element_Size bytes.
		explicit FixedSizePool(uint8_t* pBuffer)
				: m_pBuffer(pBuffer)
				, m_occupied()
		{}

	public:
		/// Gets a pointer to buffer.
		const uint8_t* buffer() const {
			return m_pBuffer;
		}

		/// Tries to allocate an entry.
		uint8_t* tryAllocate() {
			for (auto i = 0u; i < TPoolTraits::Count; ++i) {
				if (!m_occupied[i].test_and_set())
					return m_pBuffer + i * TPoolTraits::Element_Size;
			}

			return nullptr;
		}

		/// Frees a pointer belonging to this pool.
		void free(void* ptr) {
			auto diff = static_cast<size_t>(reinterpret_cast<uint8_t*>(ptr) - m_pBuffer);
			size_t index = diff / TPoolTraits::Element_Size;

			m_occupied[index].clear();
		}

	private:
		uint8_t* m_pBuffer;
		std::array<std::atomic_flag, TPoolTraits::Count> m_occupied;
	};

	/// Openssl message digest context's pool allocator.
	struct SpecializedOpensslPoolAllocator {
	public:
		static constexpr size_t Alignment = 8;
		using Align = AlignmentHelper<Alignment>;

		using Pool1 = FixedSizePool<FixedSizePoolTraits<400, Align::Size(400), 10>>;
		using Pool2 = FixedSizePool<FixedSizePoolTraits<104, Align::Size(104), 10>>;
		using Pool3 = FixedSizePool<FixedSizePoolTraits<224, Align::Size(224), 30>>;

	public:
		/// Create pool allocator.
		SpecializedOpensslPoolAllocator();

	public:
		/// Returns \c true if \a pointer belongs to this pool.
		bool isFromPool(const void* pointer) const;

		/// Copies fixed-size chunk at \a pSource to \a pDestination of \a destinationSize.
		/// \note \a pSource is expected to have been allocated from pool.
		void copyTo(void* pDestination, const void* pSource, size_t destinationSize) const;

		/// Allocate buffer of \a size bytes.
		uint8_t* allocate(size_t size);

		/// Frees \a pointer.
		/// \note \a pointer is expected to have been allocated from pool.
		void free(void* pointer);

	private:
		size_t pointerToSize(const void* pointer) const;

	private:
		std::vector<uint8_t> m_pool;
		Pool1 m_pool1;
		Pool2 m_pool2;
		Pool3 m_pool3;
	};

	/// Configures the openssl memory functions.
	void SetupOpensslMemoryFunctions();
}}
