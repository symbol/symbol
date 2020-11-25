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

#include "OpensslMemory.h"
#include <cstring>
#include <openssl/crypto.h>

namespace catapult { namespace crypto {

	namespace {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

		SpecializedOpensslPoolAllocator& GetPool() {
			static SpecializedOpensslPoolAllocator pool;
			return pool;
		}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

		void* CatMalloc(size_t num, const char*, int) {
			auto* p = GetPool().allocate(num);
			if (p)
				return p;

			return malloc(num);
		}

		void* CatRealloc(void* p, size_t newSize, const char*, int) {
			if (GetPool().isFromPool(p)) {
				auto result = malloc(newSize);
				if (!result)
					return nullptr;

				GetPool().copyTo(p, result, newSize);
				GetPool().free(p);
				return result;
			}

			return realloc(p, newSize);
		}

		void CatFree(void* p, const char*, int) {
			if (GetPool().isFromPool(p)) {
				GetPool().free(p);
				return;
			}

			free(p);
		}
	}

	SpecializedOpensslPoolAllocator::SpecializedOpensslPoolAllocator()
		: m_pool(Pool1::Size + Pool2::Size + Pool3::Size + Alignment)
		, m_pool1(Align::Pointer(m_pool.data()))
		, m_pool2(Align::Pointer(m_pool.data()) + Pool1::Size)
		, m_pool3(Align::Pointer(m_pool.data()) + Pool1::Size + Pool2::Size)
	{}

	bool SpecializedOpensslPoolAllocator::isFromPool(const void* pointer) const {
		return pointer >= m_pool1.buffer() && pointer < m_pool3.buffer() + Pool3::Size;
	}

	void SpecializedOpensslPoolAllocator::copyTo(void* pDestination, const void* pSource, size_t destinationSize) const {
		if (!destinationSize)
			return;

		auto currentSize = pointerToSize(pSource);
		std::memcpy(pDestination, pSource, std::min<size_t>(currentSize, destinationSize));
	}

	uint8_t* SpecializedOpensslPoolAllocator::allocate(size_t size) {
		switch(size) {
			case Pool1::Unaligned_Element_Size:
				return m_pool1.tryAllocate();
			case Pool2::Unaligned_Element_Size:
				return m_pool2.tryAllocate();
			case Pool3::Unaligned_Element_Size:
				return m_pool3.tryAllocate();
			default:
				return nullptr;
		}
	}

	void SpecializedOpensslPoolAllocator::free(void* pointer) {
		if (pointer < m_pool2.buffer())
			m_pool1.free(pointer);
		else if (pointer < m_pool3.buffer())
			m_pool2.free(pointer);
		else
			m_pool3.free(pointer);
	}

	size_t SpecializedOpensslPoolAllocator::pointerToSize(const void* pointer) const {
		if (pointer < m_pool2.buffer())
			return Pool1::Unaligned_Element_Size;
		else if (pointer < m_pool3.buffer())
			return Pool2::Unaligned_Element_Size;
		else
			return Pool3::Unaligned_Element_Size;
	}

	void SetupOpensslMemoryFunctions() {
		// initialize pool
		GetPool();
		CRYPTO_set_mem_functions(CatMalloc, CatRealloc, CatFree);
	}
}}
