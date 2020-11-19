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

#include "MemoryCounters.h"
#include "catapult/utils/DiagnosticCounter.h"
#include "catapult/utils/FileSize.h"
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#endif

namespace catapult { namespace local {

	namespace {
#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS GetMemoryInfo() {
			PROCESS_MEMORY_COUNTERS info;
			return GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info)) ? info : PROCESS_MEMORY_COUNTERS();
		}

#define GET_MEMORY_VALUE(NAME) utils::FileSize::FromBytes(GetMemoryInfo().NAME).megabytes()
#elif defined(__APPLE__)
		mach_task_basic_info GetMemoryInfo() {
			mach_task_basic_info info;
			mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
			return KERN_SUCCESS == task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count)
					? info
					: mach_task_basic_info();
		}

#define GET_MEMORY_VALUE(NAME) utils::FileSize::FromBytes(GetMemoryInfo().NAME).megabytes()
#else
		uint64_t GetMaxResidentSetSize() {
			rusage usage;
			return 0 == getrusage(RUSAGE_SELF, &usage)
					? utils::FileSize::FromKilobytes(static_cast<uint64_t>(usage.ru_maxrss)).megabytes()
					: 0;
		}

		struct MemoryInfo {
			uint64_t size;
			uint64_t resident;
			uint64_t shared;
		};

		MemoryInfo GetMemoryInfo() {
			MemoryInfo info;
			std::ifstream fin("/proc/self/statm");
			fin >> info.size >> info.resident >> info.shared;
			return fin ? info : MemoryInfo();
		}

		uint64_t PagesToBytes(uint64_t numPages) {
			return numPages * static_cast<uint64_t>(sysconf(_SC_PAGE_SIZE));
		}

#define GET_MEMORY_VALUE(NAME) utils::FileSize::FromBytes(PagesToBytes(GetMemoryInfo().NAME)).megabytes()
#endif

		utils::DiagnosticCounterId MakeId(const char* name) {
			return utils::DiagnosticCounterId(std::string("MEM ") + name);
		}
	}

	void AddMemoryCounters(std::vector<utils::DiagnosticCounter>& counters) {
#ifdef _WIN32
		counters.emplace_back(MakeId("CUR RSS"), []() { return GET_MEMORY_VALUE(WorkingSetSize); });
		counters.emplace_back(MakeId("MAX RSS"), []() { return GET_MEMORY_VALUE(PeakWorkingSetSize); });
		counters.emplace_back(MakeId("CUR CMT"), []() { return GET_MEMORY_VALUE(PagefileUsage); });
		counters.emplace_back(MakeId("MAX CMT"), []() { return GET_MEMORY_VALUE(PeakPagefileUsage); });
#elif defined(__APPLE__)
		counters.emplace_back(MakeId("CUR RSS"), []() { return GET_MEMORY_VALUE(resident_size); });
		counters.emplace_back(MakeId("MAX RSS"), []() { return GET_MEMORY_VALUE(resident_size_max); });
		counters.emplace_back(MakeId("CUR VIRT"), []() { return GET_MEMORY_VALUE(virtual_size); });
#else
		counters.emplace_back(MakeId("CUR RSS"), []() { return GET_MEMORY_VALUE(resident); });
		counters.emplace_back(MakeId("MAX RSS"), []() { return GetMaxResidentSetSize(); });
		counters.emplace_back(MakeId("CUR VIRT"), []() { return GET_MEMORY_VALUE(size); });
		counters.emplace_back(MakeId("SHR RSS"), []() { return GET_MEMORY_VALUE(shared); });
#endif
		}
}}
